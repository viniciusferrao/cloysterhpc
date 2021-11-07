//
// Created by Vinícius Ferrão on 31/10/21.
//
#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include "shell.h"
#include "xcat.h"

#ifdef _DEBUG_
#include <iostream>
#endif
#include <memory>
#include <fmt/format.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/process.hpp>

#include "../cluster.h"

Shell::Shell() = default;

Shell::~Shell() = default;

void Shell::runCommand(const std::string& command) {
#ifndef _DUMMY_
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
#ifdef _DEBUG_
        std::cerr << line << std::endl;
#endif

        c.wait();
#else
    std::cout << "exec: " << command << std::endl;
#endif
}

void Shell::configureSELinuxMode (Cluster::SELinuxMode mode) {

    switch (mode) {
        case Cluster::SELinuxMode::Permissive:
            runCommand("setenforce 0");
            /* Permissive mode */
            break;

        case Cluster::SELinuxMode::Enforcing:
            /* Enforcing mode */
            runCommand("setenforce 1");
            break;

        case Cluster::SELinuxMode::Disabled:
            /* Disable SELinux */
            runCommand("setenforce 0"); /* This is wrong */
            break;

        default:
            throw; /* Invalid mode */
    }
}

/* TODO: Better implementation */
void Shell::configureFirewall(bool enabled) {
    if (enabled)
        runCommand("systemctl enable --now firewalld");
    else
        runCommand("systemctl disable --now firewalld");
}

void Shell::configureFQDN (const std::string& fqdn) {
    runCommand(fmt::format("hostnamectl set hostname {}", fqdn));
}

/* TODO: Proper file parsing */
void Shell::configureHostsFile (std::string_view ip, std::string_view fqdn,
                                std::string_view hostname) {
    runCommand(fmt::format("echo {}\t{} {} >> /etc/hosts", ip, fqdn, hostname));
}

void Shell::configureTimezone (std::string timezone) {
    runCommand(fmt::format("timedatectl set timezone {}", timezone));
}

void Shell::configureLocale (std::string locale) {
    runCommand(fmt::format("localectl set locale {}", locale));
}

void Shell::runSystemUpdate (bool run) {
    if (run)
        runCommand("dnf -y update");
}

void Shell::installRequiredPackages () {
    runCommand("dnf -y install wget dnf-plugins-core");
}

void Shell::configureRepositories (const std::unique_ptr<Cluster>& cluster) {
    runCommand("dnf -y install \
        https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install \
        https://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm");

    //if (cluster->getHeadnode().m_os.id == "ol")
    runCommand("dnf config-manager --set-enabled ol8_codeready_builder");
}

/* TODO: Break into two separate functions: OpenHPC and Provision */
void Shell::installProvisioningServices () {
    runCommand("dnf -y install ohpc-base");
    runCommand("dnf -y install xCAT");
}

void Shell::configureTimeService () {
    runCommand("rpm -q chrony");
    //if not installed
    runCommand("dnf -y install chrony");

    // this should be a parse solution directly on the file instead
    runCommand("echo \"allow all\" >> /etc/chrony.conf");
    runCommand("systemctl start --now chronyd");
}

void Shell::configureSLURM (const std::unique_ptr<Cluster>& cluster) {
    runCommand("dnf -y install ohpc-slurm-server");
    runCommand("cp /etc/slurm/slurm.conf.ohpc /etc/slurm/slurm.conf");
    runCommand(fmt::format("perl -pi -e \
        \"s/ControlMachine=\\S+/ControlMachine={}/\" \
        /etc/slurm/slurm.conf", cluster->getHeadnode().getFQDN()));
}

void Shell::configureInfiniband () {
    runCommand("dnf -y groupinstall \"Infiniband Support\"");

    /* TODO: We must call the network method to configure IPoIB here */
    runCommand("cat /etc/sysconfig/network-scripts/ifcfg-ib0"); // Placeholder
}

void Shell::disableNetworkManagerDNSOverride () {
    runCommand("echo \"[main]\" > /etc/NetworkManager/conf.d/90-dns-none.conf");
    runCommand("echo \"dns=none\" >> \
        /etc/NetworkManager/conf.d/90-dns-none.conf");

    runCommand("systemctl restart NetworkManager");
}

/* TODO: Implement with NetworkManager */
void Shell::configureInternalNetwork () {
    runCommand("nmcli --help"); // Placeholder
}

void Shell::configureNetworkFileSystem () {
    runCommand("echo \"/home *(rw,no_subtree_check,fsid=10,no_root_squash)\" \
        >> /etc/exports");
    runCommand("echo \"/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\" \
        >> /etc/exports");
    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Shell::install () {
    /* TODO: Find a better way to pass the interface as a reference to prov */
    XCAT* xCAT = new XCAT(*this);

//    configureTimezone(this->m_timezone);
//    configureLocale(this->m_locale);
//    configureFQDN(this->m_headnode->m_fqdn);
//
//    this->m_firewall ? enableFirewall() : disableFirewall();
//    this->m_selinux ? configureSELinuxMode("enabled") :
//                    configureSELinuxMode("disabled");

    configureInfiniband();
    disableNetworkManagerDNSOverride();
    configureInternalNetwork();
    xCAT->setDHCPInterfaces("eth0");
    xCAT->setDomain("invalid.tld");
    xCAT->copycds("/root/OracleLinux-R8-U4-x86_64-dvd.iso");
    xCAT->genimage("ol8.4.0-x86_64-netboot-compute");
    xCAT->addOpenHPCComponents(
            "/install/netboot/ol8.4.0/x86_64/compute/rootimg/");
    configureNetworkFileSystem();

    delete xCAT;
}

void Shell::testInstall(const std::unique_ptr<Cluster>& cluster) {
    configureSELinuxMode(cluster->getSELinux());
    configureFirewall(cluster->isFirewall());
    configureFQDN(cluster->getHeadnode().getFQDN());
    configureHostsFile(cluster->getHeadnode().getConnection()
                                                    .front()
                                                    .getAddress(),
                       cluster->getHeadnode().getFQDN(),
                       cluster->getHeadnode().getHostname());
    configureTimezone(cluster->getTimezone());
    configureLocale(cluster->getLocale());
    runSystemUpdate(cluster->isUpdateSystem());

    configureRepositories(cluster);
    installRequiredPackages();

    //std::unique_ptr<Provisioner> provisioner;
    std::unique_ptr<XCAT> provisioner;
    switch (cluster->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            provisioner = std::make_unique<XCAT>(*this);
            break;
    }

    provisioner->configureRepositories();

    installProvisioningServices(); // New Implementation
    configureTimeService();
    configureSLURM(cluster);
}

