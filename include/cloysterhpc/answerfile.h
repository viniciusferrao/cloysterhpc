/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ANSWERFILE_H_
#define CLOYSTERHPC_ANSWERFILE_H_

#include <cloysterhpc/inifile.h>
#include <optional>
#include <vector>

class AnswerFile {

private:
    struct AFNetwork {
        std::optional<std::string> subnet_mask;
        std::optional<std::string> gateway;
        std::optional<std::string> domain_name;
        std::optional<std::vector<std::string>> nameservers;
        std::optional<std::string> con_interface;
        std::optional<std::string> con_ip_addr;
        std::optional<std::string> con_mac_addr;
    };

    struct AFInformation {
        std::string cluster_name;
        std::string company_name;
        std::string administrator_email;
    };

    struct AFTime {
        std::string timezone;
        std::string timeserver;
        std::string locale;
    };

    struct AFHostname {
        std::string hostname;
        std::string domain_name;
    };

    struct AFSystem {
        std::filesystem::path disk_image;
        std::string distro;
        std::string version;
        std::string kernel;
    };

    struct AFNode {
        std::optional<std::string> prefix;
        std::optional<std::string> padding;
        std::optional<std::string> start_ip;
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

    struct AFNodes {
        std::optional<AFNode> generic;
        std::vector<AFNode> nodes;
    };

    std::filesystem::path m_path;
    inifile m_ini;

    void loadOptions();
    void loadExternalNetwork();
    void loadManagementNetwork();
    void loadApplicationNetwork();
    void loadInformation();
    void loadTimeSettings();
    void loadHostnameSettings();
    void loadSystemSettings();
    void loadNodes();
    AFNode loadNode(const std::string& section);

public:
    AFNetwork external;
    AFNetwork management;
    AFNetwork application;
    AFInformation information;
    AFTime time;
    AFHostname hostname;
    AFSystem system;
    AFNodes nodes;

    explicit AnswerFile(const std::filesystem::path& path);

    [[nodiscard]] const AFNetwork& getExternal() const;
    [[nodiscard]] const AFNetwork& getManagement() const;
    [[nodiscard]] const AFNetwork& getApplication() const;
    [[nodiscard]] const AFInformation& getInformation() const;
    [[nodiscard]] const AFTime& getTime() const;
    [[nodiscard]] const AFHostname& getHostname() const;
    [[nodiscard]] const AFSystem& getSystem() const;
    [[nodiscard]] const AFNodes& getNodes() const;
};

#endif // CLOYSTERHPC_ANSWERFILE_H_
