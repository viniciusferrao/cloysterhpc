/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/services/fail2ban.h"
#include "cloysterhpc/functions.h"
#include "cloysterhpc/services/log.h"

void fail2ban::install()
{
    LOG_TRACE("Installing fail2ban");
    cloyster::runCommand("dnf -y install fail2ban-firewalld");
    configure();
}

void fail2ban::configure()
{
    //  Create the local configuration file for fail2ban
    cloyster::runCommand(
        "cat > /etc/fail2ban/jail.local << EOF\n"
        "[DEFAULT]\n"
        "# Ban IP/hosts for 24 hour ( 24h*3600s = 86400s):\n"
        "bantime = 86400\n"
        "  \n"
        "# An IP address/host is banned if it has generated \"maxretry\" "
        "during the last \"findtime\" seconds.\n"
        "findtime = 600\n"
        "maxretry = 3\n"
        "  \n"
        "# \"ignoreip\" can be a list of IP addresses, CIDR masks or DNS "
        "hosts. Fail2ban\n"
        "# will not ban a host which matches an address in this list. Several "
        "addresses\n"
        "# can be defined using space (and/or comma) separator. For example, "
        "add your\n"
        "# static IP address that you always use for login such as 103.1.2.3\n"
        "#ignoreip = 127.0.0.1/8 ::1 103.1.2.3\n"
        "  \n"
        "# Enable sshd protection\n"
        "[sshd]\n"
        "enabled = true\n"
        "EOF");
}

void fail2ban::enable() { cloyster::runCommand("systemctl enable fail2ban"); }

void fail2ban::disable() { cloyster::runCommand("systemctl disable fail2ban"); }

void fail2ban::start() { cloyster::runCommand("systemctl start fail2ban"); }

void fail2ban::stop() { cloyster::runCommand("systemctl stop fail2ban"); }
