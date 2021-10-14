/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "cluster.h"
#include "functions.h"
#include "xcat.h"

#include <iostream>
#include <boost/algorithm/string.hpp>    

void Cluster::setTimezone (std::string timezone) {
    runCommand("timedatectl set-timezone " + timezone);
}

void Cluster::setLocale (std::string locale) {
    runCommand("localectl set-locale " + locale);
}

void Cluster::setFQDN (std::string fqdn) {
    runCommand("hostnamectl set-hostname " + fqdn);
}

void Cluster::enableFirewall (void) {
    runCommand("systemctl enable --now firewalld");
}
void Cluster::disableFirewall (void) {
    runCommand("systemctl disable --now firewalld");
}

int Cluster::setSELinuxMode (std::string mode) {
    boost::to_lower(mode);

    if (mode == "enforcing" || mode == "enabled") {
        /* Enable SELinux */
        return 0;
    }

    if (mode == "permissive") {
        /* Permissive mode */
        return 0;
    }

    if (mode == "disabled") {
        /* Disable SELinux */
        return 0;
    }

    return -1; /* Failed to parse SELinux mode */
}

void Cluster::systemUpdate (void) {
    runCommand("dnf -y update");
}

void Cluster::installRequiredPackages (void) {
    runCommand("dnf -y install wget dnf-plugins-core");
}

void Cluster::setupRepositories (void) {
    runCommand("dnf -y install \
        https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install \
        http://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm");
    runCommand("wget -P /etc/yum.repos.d \
        https://xcat.org/files/xcat/repos/yum/latest/xcat-core/xcat-core.repo");
    runCommand("wget -P /etc/yum.repos.d \
        http://xcat.org/files/xcat/repos/yum/devel/xcat-dep/rh8/x86_64/xcat-dep.repo");

    //if (headnode->os.id == "ol")
    runCommand("dnf config-manager --set-enabled ol8_codeready_builder");
}

void Cluster::installProvisioningServices (void) {
    runCommand("dnf -y install ohpc-base");
    runCommand("dnf -y install xCAT");
}

void Cluster::setupTimeService (void) {
    runCommand("rpm -q chrony");
    //if not installed
    runCommand("dnf -y install chrony");

    // this should be a parse solution directly on the file instead
    runCommand("echo \"allow all\" >> /etc/chrony.conf");
    runCommand("systemctl start --now chronyd");
}

void Cluster::setupSLURM (void) {
    runCommand("dnf -y install ohpc-slurm-server");
    runCommand("cp /etc/slurm/slurm.conf.ohpc /etc/slurm/slurm.conf");
    runCommand("perl -pi -e \
        \"s/ControlMachine=\\S+/ControlMachine={HEADNODE_NAME}/\" \
        /etc/slurm/slurm.conf");
}

void Cluster::setupInfiniband (void) {
    runCommand("dnf -y groupinstall \"Infiniband Support\"");

    /* TODO: We must call the network method to configure IPoIB here */
    runCommand("cat /etc/sysconfig/network-scripts/ifcfg-ib0"); // Placeholder
}

void Cluster::disableNetworkManagerDNSOverride (void) {
    runCommand("echo \"[main]\" > /etc/NetworkManager/conf.d/90-dns-none.conf");
    runCommand("echo \"dns=none\" >> \
        /etc/NetworkManager/conf.d/90-dns-none.conf");

    runCommand("systemctl restart NetworkManager");
}

/* TODO: Implement with NetworkManager */
void Cluster::setupInternalNetwork (void) {
    runCommand("nmcli --help"); // Placeholder
}

void Cluster::setupNetworkFileSystem (void) {
    runCommand("echo \"/home *(rw,no_subtree_check,fsid=10,no_root_squash)\" \
        >> /etc/exports");
    runCommand("echo \"/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\" \
        >> /etc/exports");
    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Cluster::install (void) {
    XCAT xCAT;

    setTimezone(this->timezone);
    setLocale(this->locale);
    setFQDN(this->fqdn);

    this->firewall ? enableFirewall() : disableFirewall();
    this->selinux ? setSELinuxMode("enabled") : setSELinuxMode("disabled");

    installRequiredPackages();
    setupRepositories();
    installProvisioningServices();
    setupTimeService();
    setupSLURM();
    setupInfiniband();
    disableNetworkManagerDNSOverride();
    setupInternalNetwork();
    xCAT.setDHCPInterfaces("eth0");
    xCAT.setDomain("invalid.tld");
    xCAT.copycds("/root/OracleLinux-R8-U4-x86_64-dvd.iso");
    xCAT.genimage("ol8.4.0-x86_64-netboot-compute");
    xCAT.addOpenHPCComponents(
        "/install/netboot/ol8.4.0/x86_64/compute/rootimg/");
    setupNetworkFileSystem();
}
