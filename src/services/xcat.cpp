/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "xcat.h"
#include "execution.h"
#include "shell.h"

#include <fmt/format.h>

XCAT::XCAT(Shell& executionEngine) : m_executionEngine(executionEngine) {}

XCAT::~XCAT() = default;

void XCAT::setDHCPInterfaces (std::string interface) {
    auto command = fmt::format(
            "chdef -t site dhcpinterfaces=\"xcatmn|{0}\"", interface);
    //std::string command = "chdef -t site dhcpinterfaces=\"xcatmn|" + interface
    //    + "\"";
    
    m_executionEngine.runCommand(command);
}

void XCAT::setDomain (std::string domain) {
    std::string command = "chdef -t site domain=\"" + domain + "\"";

    m_executionEngine.runCommand(command);
}

void XCAT::copycds (std::string isopath) {
    std::string command = "copycds " + isopath;

    m_executionEngine.runCommand(command);
}

void XCAT::genimage (std::string osimage) {
    std::string command = "genimage " + osimage;

    m_executionEngine.runCommand(command);
}

void XCAT::addOpenHPCComponents (std::string chroot) {
    m_executionEngine.runCommand("cp /etc/yum.repos.d/OpenHPC.repo " + chroot +
        "/etc/yum.repos.d");
    m_executionEngine.runCommand("cp /etc/yum.repos.d/epel.repo " + chroot + "/etc/yum.repos.d");

    m_executionEngine.runCommand("dnf -y install --installroot=" + chroot +
        "install ohpc-base-compute");

    m_executionEngine.runCommand("chroot " + chroot + " systemctl disable firewalld");
}
