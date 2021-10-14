/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "xcat.h"
#include "functions.h"

void XCAT::setDHCPInterfaces (std::string interface) {
    std::string command = "chdef -t site dhcpinterfaces=\"xcatmn|" + interface
        + "\"";
    
    runCommand(command);
}

void XCAT::setDomain (std::string domain) {
    std::string command = "chdef -t site domain=\"" + domain + "\"";
    
    runCommand(command);
}

void XCAT::copycds (std::string isopath) {
    std::string command = "copycds " + isopath;

    runCommand(command);
}

void XCAT::genimage (std::string osimage) {
    std::string command = "genimage " + osimage;

    runCommand(command);
}

void XCAT::addOpenHPCComponents (std::string chroot) {
    runCommand("cp /etc/yum.repos.d/OpenHPC.repo " + chroot + 
        "/etc/yum.repos.d");
    runCommand("cp /etc/yum.repos.d/epel.repo " + chroot + "/etc/yum.repos.d");

    runCommand("dnf -y install --installroot=" + chroot + 
        "install ohpc-base-compute");

    runCommand("chroot " + chroot + " systemctl disable firewalld");
}
