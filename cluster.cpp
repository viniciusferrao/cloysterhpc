#include "cluster.hpp"
#include "functions.hpp"

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

void Cluster::install (void) {

    setTimezone(this->timezone);
    setLocale(this->locale);
    setFQDN(this->fqdn);

    this->firewall ? enableFirewall() : disableFirewall();
    this->selinux ? setSELinuxMode("enabled") : setSELinuxMode("disabled");

    installRequiredPackages();
    setupRepositories();
    installProvisioningServices();

}
