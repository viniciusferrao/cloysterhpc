/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ANSWERFILE_H_
#define CLOYSTERHPC_ANSWERFILE_H_

#include <boost/asio.hpp>
#include <cloysterhpc/mailsystem/postfix.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/ofed.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/utils/enums.h>
#include <optional>
#include <utility>
#include <vector>

using cloyster::services::Postfix;
using boost::asio::ip::address;

namespace cloyster::models {

/**
 * @struct AFNode
 * @brief Holds individual node settings.
 *
 * This structure contains optional settings for individual nodes.
 */
struct AFNode {
    std::optional<std::string> prefix;
    std::optional<std::string> padding;
    std::optional<address> start_ip = address();
    std::optional<std::string> hostname;
    std::optional<std::string> root_password;
    std::optional<std::string> mac_address;
    std::optional<std::string> sockets;
    std::optional<std::string> cores_per_socket;
    std::optional<std::string> threads_per_core;
    std::optional<std::string> bmc_address;
    std::optional<std::string> bmc_username;
    std::optional<std::string> bmc_password;
    std::optional<std::string> bmc_serialport;
    std::optional<std::string> bmc_serialspeed;
};

/**
 * @class AnswerFile
 * @brief Manages configuration settings for a cluster environment.
 *
 * This class loads, parses, and validates various configuration settings from
 * an answer file used for setting up and managing a cluster.
 */
class AnswerFile {
#ifdef BUILD_TESTING
public:
#else
private:
#endif
    /**
     * @struct AFNetwork
     * @brief Holds network configuration details.
     *
     * This structure contains optional settings related to network
     * configuration.
     */
    struct AFNetwork {
        std::optional<address> subnet_mask;
        std::optional<address> gateway;
        std::optional<std::string> domain_name;
        std::optional<std::vector<std::string>> nameservers;

        std::optional<std::string> con_interface;
        std::optional<address> con_ip_addr;
        std::optional<std::string> con_mac_addr;
    };

    /**
     * @struct AFInformation
     * @brief Holds general information settings.
     *
     * This structure contains settings for cluster name, company name, and
     * administrator email.
     */
    struct AFInformation {
        std::string cluster_name;
        std::string company_name;
        std::string administrator_email;
    };

    /**
     * @struct AFTime
     * @brief Holds time-related settings.
     *
     * This structure contains settings for timezone, timeserver, and locale.
     */
    struct AFTime {
        std::string timezone;
        std::string timeserver;
        std::string locale;
    };

    /**
     * @struct AFHostname
     * @brief Holds hostname and domain name settings.
     *
     * This structure contains the hostname and domain name settings.
     */
    struct AFHostname {
        std::string hostname;
        std::string domain_name;
    };

    /**
     * @struct AFSystem
     * @brief Holds system-related settings.
     *
     * This structure contains settings for disk image, OS distribution,
     * version, and kernel.
     */
    struct AFSystem {
        std::filesystem::path disk_image;
        OS::Distro distro;
        std::string version;
        std::string kernel;
    };

    /**
     * @struct AFNodes
     * @brief Holds settings for multiple nodes.
     *
     * This structure contains generic node settings and a list of specific node
     * settings.
     */
    struct AFNodes {
        std::optional<AFNode> generic;
        std::vector<AFNode> nodes;
    };

    struct AFPostfix {
        struct SASL {
            std::string username;
            std::string password;
        };
        struct SMTP {
            std::string server;
            int port;
            std::optional<SASL> sasl;
            // Relay doesn't need to have a specific struct because it only
            // needs 'server' and 'port'.
        };
        bool enabled = false;
        std::vector<std::string> destination;
        Postfix::Profile profile;
        std::optional<SMTP> smtp;
        std::filesystem::path cert_file;
        std::filesystem::path key_file;
    };

    struct AFOFED {
        std::string kind = cloyster::utils::enums::toString(OFED::Kind::Inbox);
        std::optional<std::string> version = "latest";
        bool enabled = false;
    };

    std::filesystem::path m_path;
    cloyster::services::files::KeyFile m_keyfile;

    /**
     * Do the inverse of `loadOptions`, i.e, move the stored settings
     * into the answerfile.
     */
    void dumpOptions();

    /**
     * @brief Loads the configuration options from the answer file.
     *
     * This function call methods to parse the answerfile and loads the
     * configuration settings.
     */
    void loadOptions();

    /**
     * @brief Converts a string to an address type.
     *
     * @param addr The string representation of the address.
     * @return The converted address.
     */
    address convertStringToAddress(const std::string& addr);

    /**
     * @brief Converts a string to multiple addresses
     *
     * @param addr The string representation of the address list.
     * @return The converted address.
     */
    std::vector<address> convertStringToMultipleAddresses(
        const std::string& addr);

    /**
     * @brief Loads the external network configuration.
     *
     * This function parses and loads the settings for the external network.
     */
    void loadExternalNetwork();

    void dumpExternalNetwork();

    /**
     * @brief Loads the management network configuration.
     *
     * This function parses and loads the settings for the management network.
     */
    void loadManagementNetwork();

    void dumpManagementNetwork();

    /**
     * @brief Loads the service network configuration.
     *
     * This function parses and loads the settings for the service network.
     */
    void loadServiceNetwork();

    /**
     * @brief Loads the application network configuration.
     *
     * This function parses and loads the settings for the application network.
     */
    void loadApplicationNetwork();

    void dumpApplicationNetwork();

    /**
     * @brief Loads the general information settings.
     *
     * This function parses and loads the cluster information settings.
     */
    void loadInformation();

    void dumpInformation();

    /**
     * @brief Loads the time-related settings.
     *
     * This function parses and loads the time settings.
     */
    void loadTimeSettings();

    void dumpTimeSettings();

    /**
     * @brief Loads the hostname settings.
     *
     * This function parses and loads the hostname settings.
     */
    void loadHostnameSettings();

    void dumpHostnameSettings();

    /**
     * @brief Loads the system settings.
     *
     * This function parses and loads the system configuration settings.
     */
    void loadSystemSettings();

    void dumpSystemSettings();

    /**
     * @brief Loads the node settings.
     *
     * This function parses and loads the settings for all nodes.
     */
    void loadNodes();
    void loadTools();
    void loadNVHPC();
    void loadOFED();

    void dumpNodes();

    void loadPostfix();
    void dumpPostfix();

    bool checkEnabled(const std::string& section);
    /**
     * @brief Loads the settings for a specific node.
     *
     * @param section The section in the answer file representing the node.
     * @return The node settings.
     */
    AFNode loadNode(const std::string& section);

    /**
     * @brief Validates the settings for a node.
     *
     * @param node The node to validate.
     * @return The validated node settings.
     */
    AFNode validateNode(AFNode node);

    /**
     * @brief Validates an attribute against a generic attribute.
     *
     * @tparam T The type of the attribute.
     * @param sectionName The name of the section containing the attribute.
     * @param attributeName The name of the attribute.
     * @param objectAttr The attribute to validate.
     * @param genericAttr The generic attribute to validate against.
     */
    template <typename T>
    void validateAttribute(const std::string& sectionName,
        const std::string& attributeName, T& objectAttr, const T& genericAttr);

    /**
     * @brief Converts a network address and validates it.
     *
     * @tparam T The type of the network address.
     * @param section The section containing the address.
     * @param fieldName The name of the field representing the address.
     * @param destination The destination to store the converted address.
     * @param isOptional Indicates if the field is optional.
     */
    template <typename T>
    void convertNetworkAddressAndValidate(const std::string& section,
        const std::string& fieldName, T& destination, bool isOptional = true);

    /**
     * @brief Loads the network configuration.
     *
     * @tparam NetworkType The type of the network configuration.
     * @param networkSection The section in the answer file representing the
     * network.
     * @param network The network configuration to load.
     * @param optionalNameservers Indicates if the nameservers are optional.
     */
    void loadNetwork(const std::string& networkSection, AFNetwork& network);

    void dumpNetwork(
        const AFNetwork& network, const std::string& networkSection);

public:
    AFNetwork external;
    AFNetwork management;
    AFNetwork application;
    AFNetwork service;
    AFInformation information;
    AFTime time;
    AFHostname hostname;
    AFSystem system;
    AFNodes nodes;
    AFPostfix postfix;
    AFOFED ofed;

    /**
     * @brief Loads the answer file from the specified path.
     *
     * @param path The path to the answer file.
     */
    void loadFile(const std::filesystem::path& path);
    void dumpFile(const std::filesystem::path& path);

    AnswerFile();
    explicit AnswerFile(const std::filesystem::path& path);
};

class AnswerfileValidationException : public std::exception {
private:
    std::string message;

public:
    AnswerfileValidationException(const AnswerfileValidationException&)
        = default;
    AnswerfileValidationException(AnswerfileValidationException&&) = delete;
    AnswerfileValidationException& operator=(
        const AnswerfileValidationException&)
        = default;
    AnswerfileValidationException& operator=(AnswerfileValidationException&&)
        = delete;

    explicit AnswerfileValidationException(const char* msg)
        : message(msg)
    {
    }

    explicit AnswerfileValidationException(std::string msg)
        : message(std::move(msg))
    {
    }

    ~AnswerfileValidationException() override = default;

    [[nodiscard]] const char* what() const noexcept override
    {
        return message.c_str();
    }
};

};

#endif // CLOYSTERHPC_ANSWERFILE_H_
