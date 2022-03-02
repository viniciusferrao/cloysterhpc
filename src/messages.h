/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef MESSAGES_H
#define MESSAGES_H

constexpr const char * MSG_TITLE_DIRECTORY_SERVICES_SETTINGS = \
    "Directory Settings\n";
constexpr const char * MSG_TITLE_NODE_SETTINGS = \
    "Compute Node Settings\n";

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

constexpr const char * MSG_DEFAULT_HELP_NOT_AVAILABLE = \
    "We're sorry, but there's no help available for this screen :(\n";

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

#endif /* MESSAGES_H */
