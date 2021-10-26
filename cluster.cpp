/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "cluster.h"
#include "functions.h"
#include "headnode.h"
#include "xcat.h"

#include <iostream>
#include <boost/algorithm/string.hpp>

Cluster::Cluster () {
    m_headnode = new Headnode();
}

Cluster::Cluster (Headnode& headnode) {
    m_headnode = &headnode;
}

Cluster::~Cluster () {
    delete m_headnode;
}

void Cluster::setTimezone (std::string timezone) {
    runCommand("timedatectl set-timezone " + timezone);
}

void Cluster::setLocale (std::string locale) {
    runCommand("localectl set-locale " + locale);
}

void Cluster::setFQDN (const std::string& fqdn) {
    runCommand("hostnamectl set-hostname " + fqdn);
}

void Cluster::enableFirewall () {
    runCommand("systemctl enable --now firewalld");
}
void Cluster::disableFirewall () {
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

void Cluster::systemUpdate () {
    runCommand("dnf -y update");
}

void Cluster::installRequiredPackages () {
    runCommand("dnf -y install wget dnf-plugins-core");
}

void Cluster::setupRepositories () {
    runCommand("dnf -y install \
        https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install \
        https://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm");
    runCommand("wget -P /etc/yum.repos.d \
        https://xcat.org/files/xcat/repos/yum/latest/xcat-core/xcat-core.repo");
    runCommand("wget -P /etc/yum.repos.d \
        https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/rh8/x86_64/xcat-dep.repo");

    //if (headnode->os.id == "ol")
    runCommand("dnf config-manager --set-enabled ol8_codeready_builder");
}

void Cluster::installProvisioningServices () {
    runCommand("dnf -y install ohpc-base");
    runCommand("dnf -y install xCAT");
}

void Cluster::setupTimeService () {
    runCommand("rpm -q chrony");
    //if not installed
    runCommand("dnf -y install chrony");

    // this should be a parse solution directly on the file instead
    runCommand("echo \"allow all\" >> /etc/chrony.conf");
    runCommand("systemctl start --now chronyd");
}

void Cluster::setupSLURM () {
    runCommand("dnf -y install ohpc-slurm-server");
    runCommand("cp /etc/slurm/slurm.conf.ohpc /etc/slurm/slurm.conf");
    runCommand("perl -pi -e \
        \"s/ControlMachine=\\S+/ControlMachine={HEADNODE_NAME}/\" \
        /etc/slurm/slurm.conf");
}

void Cluster::setupInfiniband () {
    runCommand("dnf -y groupinstall \"Infiniband Support\"");

    /* TODO: We must call the network method to configure IPoIB here */
    runCommand("cat /etc/sysconfig/network-scripts/ifcfg-ib0"); // Placeholder
}

void Cluster::disableNetworkManagerDNSOverride () {
    runCommand("echo \"[main]\" > /etc/NetworkManager/conf.d/90-dns-none.conf");
    runCommand("echo \"dns=none\" >> \
        /etc/NetworkManager/conf.d/90-dns-none.conf");

    runCommand("systemctl restart NetworkManager");
}

/* TODO: Implement with NetworkManager */
void Cluster::setupInternalNetwork () {
    runCommand("nmcli --help"); // Placeholder
}

void Cluster::setupNetworkFileSystem () {
    runCommand("echo \"/home *(rw,no_subtree_check,fsid=10,no_root_squash)\" \
        >> /etc/exports");
    runCommand("echo \"/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\" \
        >> /etc/exports");
    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Cluster::install () {
    XCAT xCAT;

    setTimezone(this->timezone);
    setLocale(this->locale);
    setFQDN(this->m_headnode->fqdn);

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

#ifdef _DEBUG_
void Cluster::printData () {
    std::cout << "Cluster attributes defined:" << std::endl;
    std::cout << "Timezone: " << timezone << std::endl;
    std::cout << "Locale: " << locale << std::endl;
    std::cout << "Hostname: " << this->m_headnode->hostname << std::endl;
    std::cout << "Domainname: " << domainname << std::endl;
    std::cout << "FQDN: " << this->m_headnode->fqdn << std::endl;
    //std::cout << "interfaceExternal: " << cluster.interfaceExternal << std::endl;
    //std::cout << "interfaceInternal: " << cluster.interfaceInternal << std::endl;
    //std::cout << "interfaceInternalNetwork: " << cluster.interfaceInternalNetwork << std::endl;
    //std::cout << "interfaceInternalIP: " << cluster.interfaceInternalIP << std::endl;
    std::cout << "xCATDynamicRangeStart: " << xCATDynamicRangeStart << std::endl;
    std::cout << "xCATDynamicRangeEnd: " << xCATDynamicRangeEnd << std::endl;

    std::cout << "Directory Admin Password: " << directoryAdminPassword << std::endl;
    std::cout << "Directory Manager Password: " << directoryManagerPassword << std::endl;
    std::cout << "Directory Disable DNSSEC: " << (directoryDisableDNSSEC ? "true" : "false") << std::endl;

    std::cout << "nodePrefix: " << nodePrefix << std::endl;
    std::cout << "nodePadding: " << nodePadding << std::endl;
    std::cout << "nodeStartIP: " << nodeStartIP << std::endl;
    std::cout << "nodeRootPassword: " << nodeRootPassword << std::endl;
    std::cout << "nodeISOPath: " << nodeISOPath << std::endl;

    std::cout << "ibStack: " << ibStack << std::endl;

    std::cout << "queueSystem: " << queueSystem.name << std::endl;
    // if (queueSystem.name == "SLURM")
    //     std::cout << "slurm.partition: " << queueSystem.slurm.partition << std::endl;
    // if (queueSystem.name == "PBS")
    //     std::cout << "pbs.defaultPlace: " << queueSystem.pbs.defaultPlace << std::endl;

    // std::cout << "Enable Postfix: " << postfix.enable ? "true" : "false" << std::endl;
    // if (postfix.enable) {
    //     std::cout << "\t-> Profile: " << postfixProfiles[cluster.postfix.profileId] << std::endl;
    //     switch (cluster.postfix.profileId) {
    //         case 0:
    //             /* Local */
    //             break;
    //         case 1:
    //             /* Relay */
    //             std::cout << "\t\t-> Hostname: " << cluster.postfix.relay.hostname << std::endl;
    //             std::cout << "\t\t-> Port: %u\n", cluster.postfix.relay.port << std::endl;
    //             break;
    //         case 2:
    //             /* SASL */
    //             std::cout << "\t\t-> Hostname: " << cluster.postfix.sasl.hostname << std::endl;
    //             std::cout << "\t\t-> Port: %u\n", cluster.postfix.sasl.port << std::endl;
    //             std::cout << "\t\t-> Username: " << cluster.postfix.sasl.username << std::endl;
    //             std::cout << "\t\t-> Password: " << cluster.postfix.sasl.password << std::endl;
    //             break;
    //     }
    // }

    std::cout << "Update system: " << (updateSystem ? "true" : "false") << std::endl;
    std::cout << "Remote access: " << (remoteAccess ? "true" : "false") << std::endl;

    std::cout << "Firewall: " << (firewall ? "true" : "false") << std::endl;
    std::cout << "SELinux: " << (selinux ? "true" : "false") << std::endl;
}
#endif
