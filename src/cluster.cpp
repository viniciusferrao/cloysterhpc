/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "cluster.h"
#include "functions.h"
#include "headnode.h"
#include "services/xcat.h"

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

#ifdef _DEBUG_
void Cluster::printData () {
    std::cerr << "Cluster attributes defined:" << std::endl;
    std::cerr << "Timezone: " << timezone << std::endl;
    std::cerr << "Locale: " << locale << std::endl;
    std::cerr << "Hostname: " << this->m_headnode->hostname << std::endl;
    std::cerr << "Domainname: " << domainname << std::endl;
    std::cerr << "FQDN: " << this->m_headnode->fqdn << std::endl;
    //std::cerr << "interfaceExternal: " << cluster.interfaceExternal << std::endl;
    //std::cerr << "interfaceInternal: " << cluster.interfaceInternal << std::endl;
    //std::cerr << "interfaceInternalNetwork: " << cluster.interfaceInternalNetwork << std::endl;
    //std::cerr << "interfaceInternalIP: " << cluster.interfaceInternalIP << std::endl;
    std::cerr << "xCATDynamicRangeStart: " << xCATDynamicRangeStart << std::endl;
    std::cerr << "xCATDynamicRangeEnd: " << xCATDynamicRangeEnd << std::endl;

    std::cerr << "Directory Admin Password: " << directoryAdminPassword << std::endl;
    std::cerr << "Directory Manager Password: " << directoryManagerPassword << std::endl;
    std::cerr << "Directory Disable DNSSEC: " << (directoryDisableDNSSEC ? "true" : "false") << std::endl;

    std::cerr << "nodePrefix: " << nodePrefix << std::endl;
    std::cerr << "nodePadding: " << nodePadding << std::endl;
    std::cerr << "nodeStartIP: " << nodeStartIP << std::endl;
    std::cerr << "nodeRootPassword: " << nodeRootPassword << std::endl;
    std::cerr << "nodeISOPath: " << nodeISOPath << std::endl;

    std::cerr << "ibStack: " << ibStack << std::endl;

    std::cerr << "queueSystem: " << queueSystem.name << std::endl;
    // if (queueSystem.name == "SLURM")
    //     std::cerr << "slurm.partition: " << queueSystem.slurm.partition << std::endl;
    // if (queueSystem.name == "PBS")
    //     std::cerr << "pbs.defaultPlace: " << queueSystem.pbs.defaultPlace << std::endl;

    // std::cerr << "Enable Postfix: " << postfix.enable ? "true" : "false" << std::endl;
    // if (postfix.enable) {
    //     std::cerr << "\t-> Profile: " << postfixProfiles[cluster.postfix.profileId] << std::endl;
    //     switch (cluster.postfix.profileId) {
    //         case 0:
    //             /* Local */
    //             break;
    //         case 1:
    //             /* Relay */
    //             std::cerr << "\t\t-> Hostname: " << cluster.postfix.relay.hostname << std::endl;
    //             std::cerr << "\t\t-> Port: %u\n", cluster.postfix.relay.port << std::endl;
    //             break;
    //         case 2:
    //             /* SASL */
    //             std::cerr << "\t\t-> Hostname: " << cluster.postfix.sasl.hostname << std::endl;
    //             std::cerr << "\t\t-> Port: %u\n", cluster.postfix.sasl.port << std::endl;
    //             std::cerr << "\t\t-> Username: " << cluster.postfix.sasl.username << std::endl;
    //             std::cerr << "\t\t-> Password: " << cluster.postfix.sasl.password << std::endl;
    //             break;
    //     }
    // }

    std::cerr << "Update system: " << (updateSystem ? "true" : "false") << std::endl;
    std::cerr << "Remote access: " << (remoteAccess ? "true" : "false") << std::endl;

    std::cerr << "Firewall: " << (firewall ? "true" : "false") << std::endl;
    std::cerr << "SELinux: " << (selinux ? "true" : "false") << std::endl;
}

void Cluster::fillTestData () {
    firewall = true;
    selinux = true;
    timezone = "America/Sao_Paulo";
    locale = "en_US.UTF-8";
    this->m_headnode->hostname = "headnode";
    domainname = "cluster.example.tld";
    this->m_headnode->fqdn = this->m_headnode->hostname + "." + domainname;
    xCATDynamicRangeStart = "192.168.20.1";
    xCATDynamicRangeEnd = "192.168.20.254";
    directoryAdminPassword = "pwdAdmin";
    directoryManagerPassword = "pwdManager";
    directoryDisableDNSSEC = true;
    nodePrefix = "n";
    nodePadding = "2";
    nodeStartIP = "172.26.0.1";
    nodeRootPassword = "pwdNodeRoot";
    nodeISOPath = "/mnt/iso/rhel-8.4-dvd.iso";
    ibStack = "MLNX";
    queueSystem.name = "SLURM";
    updateSystem = true;
    remoteAccess = true;
}

#endif
