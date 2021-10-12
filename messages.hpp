/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef _HPP_MESSAGES_
#define _HPP_MESSAGES_

//constexpr const char * MSG_INSTALL_HELP_LINE "  <Tab>/<Alt-Tab> between elements   |  <Space> selects   |  <F12> disabled"
#define MSG_INSTALL_HELP_LINE NULL

constexpr const char * MSG_INSTALL_TITLE = \
    "CloysterHPC Installer\n";
constexpr const char * MSG_INSTALL_ABORT = \
    "Installation aborted due to operator request\n";

constexpr const char * MSG_TITLE_HELP = \
    "Help\n";
constexpr const char * MSG_TITLE_TIME_SETTINGS = \
    "Timezone Settings\n";
constexpr const char * MSG_TITLE_LOCALE_SETTINGS = \
    "Locale Settings\n";
constexpr const char * MSG_TITLE_NETWORK_SETTINGS = \
    "Network Settings\n";
constexpr const char * MSG_TITLE_DIRECTORY_SERVICES_SETTINGS = \
    "Directory Settings\n";
constexpr const char * MSG_TITLE_NODE_SETTINGS = \
    "Compute Node Settings\n";
constexpr const char * MSG_TITLE_INFINIBAND_SETTINGS = \
    "Infiniband Settings\n";
constexpr const char * MSG_TITLE_QUEUE_SYSTEM_SETTINGS = \
    "Queue System Settings\n";
constexpr const char * MSG_TITLE_SLURM_SETTINGS = \
    "SLURM Settings\n";
constexpr const char * MSG_TITLE_PBS_SETTINGS = \
    "PBS Professional Settings\n";
constexpr const char * MSG_TITLE_POSTFIX_SETTINGS = \
    "Postfix Settings\n";
constexpr const char * MSG_TITLE_UPDATE_SYSTEM = \
    "Update\n";
constexpr const char * MSG_TITLE_REMOTE_ACCESS = \
    "Remote Support\n";

constexpr const char * MSG_BUTTON_OK = "Ok\n";
constexpr const char * MSG_BUTTON_CANCEL = "Cancel\n";
constexpr const char * MSG_BUTTON_HELP = "Help\n";
constexpr const char * MSG_BUTTON_YES = "Yes\n";
constexpr const char * MSG_BUTTON_NO = "No\n";

constexpr const char * MSG_WELCOME = \
    "Welcome to the guided installer of CloysterHPC!\n\nLorem ipsum dolor " \
    "sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt " \
    "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. " \
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum " \
    "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non " \
    "proident, sunt in culpa qui officia deserunt mollit anim id est laborum." \
    "\n";
constexpr const char * MSG_GUIDED_INSTALL = \
    "We will start the installation phase now.\n\nYou will be guided through " \
    "a series of questions about your HPC cluster so we can configure " \
    "everything accordingly to your needs. You're free to cancel the process " \
    "at any given time.\n";

constexpr const char * MSG_DEFAULT_HELP_NOT_AVAILABLE = \
    "We're sorry, but there's no help available for this screen :(\n";

constexpr const char * MSG_TIME_SETTINGS_TIMEZONE = \
    "Choose you local timezone\n";
constexpr const char * MSG_TIME_SETTINGS_TIMEZONE_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_LOCALE_SETTINGS_LOCALE = \
    "Pick your default locale\n";
constexpr const char * MSG_LOCALE_SETTINGS_LOCALE_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_NETWORK_SETTINGS_HOSTID = \
    "Enter the desired hostname and domain name for this machine\n";
constexpr const char * MSG_NETWORK_SETTINGS_HOSTID_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_NETWORK_SETTINGS_EXTERNAL_IF = \
    "Select your external network interface\n";
constexpr const char * MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_NETWORK_SETTINGS_INTERNAL_IF = \
    "Select your internal network interface\n";
constexpr const char * MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_NETWORK_SETTINGS_INTERNAL_IPV4 = \
    "Enter the IPv4 information for the management network\n";
constexpr const char * MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE = \
    "Enter the xCAT Dynamic range for node discovery\n";
constexpr const char * MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_NETWORK_SETTINGS_IP = \
    "Fill the IP network information\n";
constexpr const char * MSG_NETWORK_SETTINGS_IP_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD = \
    "Enter the admin and directory manager passwords\n";
constexpr const char * MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC = \
    "Disable DNSSEC validation on this directory server?\n";
constexpr const char * MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_NODE_SETTINGS = \
    "Enter the compute nodes information\n";
constexpr const char * MSG_NODE_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_INFINIBAND_SETTINGS = \
    "Chose the Infiniband stack to be executed\n";
constexpr const char * MSG_INFINIBAND_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_QUEUE_SYSTEM_SETTINGS = \
    "Pick a queue system to run you compute jobs\n";
constexpr const char * MSG_QUEUE_SYSTEM_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_SLURM_SETTINGS = \
    "Entry the default partition name for SLURM\n";
constexpr const char * MSG_SLURM_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_PBS_SETTINGS = \
    "Entry the default execution place for PBS Professional jobs\n";
constexpr const char * MSG_PBS_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_POSTFIX_ENABLE = \
    "Do you want to enable Postfix?\n";
constexpr const char * MSG_POSTFIX_ENABLE_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_POSTFIX_SETTINGS_PROFILE = \
    "Choose a profile for mail delivery\n";
constexpr const char * MSG_POSTFIX_SETTINGS_PROFILE_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_POSTFIX_RELAY_SETTINGS = \
    "Enter the destination MTA information to relay messages\n";
constexpr const char * MSG_POSTFIX_RELAY_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;
constexpr const char * MSG_POSTFIX_SASL_SETTINGS = \
    "Enter the mail server and user information to deliver messages\n";
constexpr const char * MSG_POSTFIX_SASL_SETTINGS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_UPDATE_SYSTEM = \
    "Update the system before install?\n";
constexpr const char * MSG_UPDATE_SYSTEM_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;

constexpr const char * MSG_REMOTE_ACCESS = \
    "Enable remote access to provide additional support?\n";
constexpr const char * MSG_REMOTE_ACCESS_HELP = \
    MSG_DEFAULT_HELP_NOT_AVAILABLE;   

#ifdef _DEPRECATED_
constexpr const char * MSG_DEFINE_HOSTNAME = \
    "Enter the hostname of the control machine:\n";

constexpr const char * MSG_DEFINE_DOMAINNAME = \
    "Enter the domain name of the cluster to be used:\n";
#endif

#endif /* _HPP_MESSAGES_ */
