/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef _H_MESSAGES_
#define _H_MESSAGES_

//#define MSG_INSTALL_HELP_LINE "  <Tab>/<Alt-Tab> between elements   |  <Space> selects   |  <F12> disabled"
#define MSG_INSTALL_HELP_LINE NULL
#define MSG_INSTALL_TITLE "CloysterHPC Installer\n"
#define MSG_INSTALL_ABORT "Installation aborted due to operator request\n"

#define MSG_TITLE_HELP "Help\n"
#define MSG_TITLE_TIME_SETTINGS "Timezone Settings\n"
#define MSG_TITLE_LOCALE_SETTINGS "Locale Settings\n"
#define MSG_TITLE_NETWORK_SETTINGS "Network Settings\n"
#define MSG_TITLE_DIRECTORY_SERVICES_SETTINGS "Directory Settings\n"
#define MSG_TITLE_NODE_SETTINGS "Compute Node Settings\n"
#define MSG_TITLE_INFINIBAND_SETTINGS "Infiniband Settings\n"
#define MSG_TITLE_QUEUE_SYSTEM_SETTINGS "Queue System Settings\n"
#define MSG_TITLE_SLURM_SETTINGS "SLURM Settings\n"
#define MSG_TITLE_PBS_SETTINGS "PBS Professional Settings\n"
#define MSG_TITLE_POSTFIX_SETTINGS "Postfix Settings\n"
#define MSG_TITLE_UPDATE_SYSTEM "Update\n"
#define MSG_TITLE_REMOTE_ACCESS "Remote Support\n"

#define MSG_BUTTON_OK "Ok\n"
#define MSG_BUTTON_CANCEL "Cancel\n"
#define MSG_BUTTON_HELP "Help\n"
#define MSG_BUTTON_YES "Yes\n"
#define MSG_BUTTON_NO "No\n"

#define MSG_WELCOME \
    "Welcome to the guided installer of CloysterHPC!\n\nLorem ipsum dolor " \
    "sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt " \
    "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. " \
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum " \
    "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non " \
    "proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
#define MSG_GUIDED_INSTALL \
    "We will start the installation phase now.\n\nYou will be guided through " \
    "a series of questions about your HPC cluster so we can configure " \
    "everything accordingly to your needs. You're free to cancel the process " \
    "at any given time.\n"

#define MSG_TIME_SETTINGS_TIMEZONE \
    "Choose you local timezone\n"
#define MSG_TIME_SETTINGS_TIMEZONE_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_LOCALE_SETTINGS_LOCALE \
    "Pick your default locale\n"
#define MSG_LOCALE_SETTINGS_LOCALE_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_NETWORK_SETTINGS_HOSTID \
    "Enter the desired hostname and domain name for this machine\n"
#define MSG_NETWORK_SETTINGS_HOSTID_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_NETWORK_SETTINGS_EXTERNAL_IF \
    "Select your external network interface\n"
#define MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_NETWORK_SETTINGS_INTERNAL_IF \
    "Select your internal network interface\n"
#define MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_NETWORK_SETTINGS_INTERNAL_IPV4 \
    "Enter the IPv4 information for the management network\n"
#define MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE \
    "Enter the xCAT Dynamic range for node discovery\n"
#define MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD \
    "Enter the admin and directory manager passwords\n"
#define MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC \
    "Disable DNSSEC validation on this directory server?\n"
#define MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_NODE_SETTINGS \
    "Enter the compute nodes information\n"
#define MSG_NODE_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_INFINIBAND_SETTINGS \
    "Chose the Infiniband stack to be executed\n"
#define MSG_INFINIBAND_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_QUEUE_SYSTEM_SETTINGS \
    "Pick a queue system to run you compute jobs\n"
#define MSG_QUEUE_SYSTEM_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_SLURM_SETTINGS \
    "Entry the default partition name for SLURM\n"
#define MSG_SLURM_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_PBS_SETTINGS \
    "Entry the default execution place for PBS Professional jobs\n"
#define MSG_PBS_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_POSTFIX_ENABLE \
    "Do you want to enable Postfix?\n"
#define MSG_POSTFIX_ENABLE_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_POSTFIX_SETTINGS_PROFILE \
    "Choose a profile for mail delivery\n"
#define MSG_POSTFIX_SETTINGS_PROFILE_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_POSTFIX_RELAY_SETTINGS \
    "Enter the destination MTA information to relay messages\n"
#define MSG_POSTFIX_RELAY_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"
#define MSG_POSTFIX_SASL_SETTINGS \
    "Enter the mail server and user information to deliver messages\n"
#define MSG_POSTFIX_SASL_SETTINGS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_UPDATE_SYSTEM \
    "Update the system before install?\n"
#define MSG_UPDATE_SYSTEM_HELP \
    "We're sorry, but there's no help available for this screen :(\n"

#define MSG_REMOTE_ACCESS \
    "Enable remote access to provide additional support?\n"
#define MSG_REMOTE_ACCESS_HELP \
    "We're sorry, but there's no help available for this screen :(\n"   

#ifdef _DEPRECATED_
#define MSG_DEFINE_HOSTNAME \
    "Enter the hostname of the control machine:\n"

#define MSG_DEFINE_DOMAINNAME \
    "Enter the domain name of the cluster to be used:\n"
#endif

#endif /* _H_MESSAGES_ */
