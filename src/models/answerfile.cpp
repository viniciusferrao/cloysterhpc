/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/answerfile.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cstddef>
#include <fmt/core.h>
#include <iterator>
#include <ranges>

using cloyster::services::Postfix;

namespace cloyster::models {

AnswerFile::AnswerFile(const std::filesystem::path& path)
    : m_path(path)
    , m_keyfile(path)
{
    loadFile(m_path);
}

// AnswerFile::AnswerFile() {};

void AnswerFile::loadFile(const std::filesystem::path& path)
{
    m_path = path;
    m_keyfile.load();
    loadOptions();
};

void AnswerFile::loadOptions()
{
    LOG_TRACE("Verify answerfile variables")

    loadExternalNetwork();
    loadManagementNetwork();
    loadApplicationNetwork();
    loadInformation();
    loadTimeSettings();
    loadHostnameSettings();
    loadSystemSettings();
    loadNodes();
    loadPostfix();
    loadOFED();
}

void AnswerFile::dumpNetwork(
    const AFNetwork& network, const std::string& networkSection)
{
    if (network.con_interface) {
        m_keyfile.setString(
            networkSection, "interface", network.con_interface.value());
    }

    if (network.con_ip_addr) {
        m_keyfile.setString(
            networkSection, "ip_address", network.con_ip_addr->to_string());
    }

    if (network.con_mac_addr) {
        m_keyfile.setString(
            networkSection, "mac_address", *network.con_mac_addr);
    }

    if (network.subnet_mask) {
        m_keyfile.setString(
            networkSection, "subnet_mask", network.subnet_mask->to_string());
    }

    if (network.domain_name) {
        m_keyfile.setString(
            networkSection, "domain_name", *network.domain_name);
    }

    if (network.gateway) {
        m_keyfile.setString(
            networkSection, "gateway", network.gateway->to_string());
    }

    if (network.nameservers && !network.nameservers->empty()) {
        auto nameval = boost::algorithm::join(*network.nameservers, ", ");
        m_keyfile.setString(networkSection, "nameservers", nameval);
    }
}

void AnswerFile::dumpExternalNetwork()
{
    dumpNetwork(external, "network_external");
}

void AnswerFile::dumpManagementNetwork()
{
    dumpNetwork(management, "network_management");
}

void AnswerFile::dumpApplicationNetwork()
{
    dumpNetwork(application, "network_application");
}

void AnswerFile::dumpInformation()
{
    m_keyfile.setString(
        "information", "cluster_name", information.cluster_name);
    m_keyfile.setString(
        "information", "company_name", information.company_name);
    m_keyfile.setString("information", "admm_keyfilestrator_email",
        information.administrator_email);
}

void AnswerFile::dumpTimeSettings()
{
    m_keyfile.setString("time", "timezone", time.timezone);
    m_keyfile.setString("time", "timeserver", time.timeserver);
    m_keyfile.setString("time", "locale", time.locale);
}

void AnswerFile::dumpHostnameSettings()
{
    m_keyfile.setString("hostname", "hostname", hostname.hostname);
    m_keyfile.setString("hostname", "domain_name", hostname.domain_name);
}

void AnswerFile::dumpSystemSettings()
{
    auto distroName = cloyster::utils::enums::toString(system.distro);

    m_keyfile.setString("system", "disk_image", system.disk_image.string());
    m_keyfile.setString("system", "distro", std::string { distroName });
    m_keyfile.setString("system", "version", system.version);
    m_keyfile.setString("system", "kernel", system.kernel);
}

void AnswerFile::dumpNodes()
{
    size_t counter = 1;
    for (const auto& node : nodes.nodes) {
        std::string sectionName = fmt::format("node.{}", counter);

        if (node.hostname) {
            m_keyfile.setString(sectionName, "hostname", *node.hostname);
        }

        if (node.root_password) {
            m_keyfile.setString(
                sectionName, "node_root_password", *node.root_password);
        }

        if (node.sockets) {
            m_keyfile.setString(sectionName, "sockets", *node.sockets);
        }

        if (node.cores_per_socket) {
            m_keyfile.setString(
                sectionName, "cores_per_socket", *node.cores_per_socket);
        }

        if (node.threads_per_core) {
            m_keyfile.setString(
                sectionName, "threads_per_core", *node.threads_per_core);
        }

        if (node.bmc_address) {
            m_keyfile.setString(sectionName, "bmc_address", *node.bmc_address);
        }

        if (node.bmc_username) {
            m_keyfile.setString(
                sectionName, "bmc_username", *node.bmc_username);
        }

        if (node.bmc_password)
            m_keyfile.setString(
                sectionName, "bmc_password", *node.bmc_password);

        if (node.bmc_serialport)
            m_keyfile.setString(
                sectionName, "bmc_serialport", *node.bmc_serialport);

        if (node.bmc_serialspeed)
            m_keyfile.setString(
                sectionName, "bmc_serialspeed", *node.bmc_serialspeed);

        counter++;
    }
}

void AnswerFile::dumpPostfix()
{
    if (!postfix.enabled) {
        return;
    }

    auto profileName = cloyster::utils::enums::toString(postfix.profile);
    m_keyfile.setString("postfix", "profile", std::string { profileName });
    m_keyfile.setString(
        "postfix", "smtpd_tls_cert_file", postfix.cert_file.string());
    m_keyfile.setString(
        "postfix", "smtpd_tls_key_file", postfix.key_file.string());

    switch (postfix.profile) {
        case Postfix::Profile::Local:
            break;
        case Postfix::Profile::Relay:
            m_keyfile.setString(
                "postfix.relay", "server", postfix.smtp->server);
            m_keyfile.setString(
                "postfix.relay", "port", std::to_string(postfix.smtp->port));
            break;
        case Postfix::Profile::SASL:
            m_keyfile.setString("postfix.sasl", "server", postfix.smtp->server);
            m_keyfile.setString(
                "postfix.sasl", "port", std::to_string(postfix.smtp->port));
            m_keyfile.setString(
                "postfix.sasl", "username", postfix.smtp->sasl->username);
            m_keyfile.setString(
                "postfix.sasl", "password", postfix.smtp->sasl->password);
            break;
    }
}

void AnswerFile::dumpOptions()
{
    LOG_TRACE("Dump answerfile variables")

    dumpExternalNetwork();
    dumpManagementNetwork();
    dumpApplicationNetwork();
    dumpInformation();
    dumpTimeSettings();
    dumpHostnameSettings();
    dumpSystemSettings();

    dumpNodes();
    dumpPostfix();
}

void AnswerFile::dumpFile(const std::filesystem::path& path)
{
    dumpOptions();
    m_keyfile.save();
};

address AnswerFile::convertStringToAddress(const std::string& addr)
{
    try {
        return boost::asio::ip::make_address(addr);
    } catch (boost::system::system_error& e) {
        throw std::invalid_argument("Invalid address");
    }
}

std::vector<address> AnswerFile::convertStringToMultipleAddresses(
    const std::string& addr)
{
    std::vector<address> out;
    std::vector<std::string> strout;
    try {
        boost::split(strout, addr, boost::is_any_of(","));
    } catch (boost::system::system_error& e) {
        throw std::invalid_argument(
            "Invalid character while decoding multiple addresses");
    }

    std::transform(strout.begin(), strout.end(), std::back_inserter(out),
        [this](auto& s) { return this->convertStringToAddress(s); });

    return out;
}

template <typename T>
void AnswerFile::validateAttribute(const std::string& sectionName,
    const std::string& attributeName, T& objectAttr, const T& genericAttr)
{
    if constexpr (std::is_same_v<T, std::optional<std::basic_string<char>>>) {
        if (!objectAttr->empty()) {
            return;
        }

        if (genericAttr->empty()) {
            throw std::invalid_argument(
                fmt::format("{1} must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    attributeName, sectionName));
        }

        objectAttr = genericAttr;
    } else if constexpr (std::is_same_v<T,
                             std::optional<boost::asio::ip::address>>) {
        if (!objectAttr->is_unspecified()) {
            return;
        }

        if (genericAttr->is_unspecified()) {
            throw std::invalid_argument(
                fmt::format("{1} must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    attributeName, sectionName));
        }

        objectAttr = genericAttr;
    } else {
        throw std::invalid_argument("Unsupported type for validateAttribute");
    }
}

template <typename T>
void AnswerFile::convertNetworkAddressAndValidate(
    const std::string& sectionName, const std::string& fieldName,
    T& destination, bool isOptional)
{
    if (!m_keyfile.getStringOpt(sectionName, fieldName)) {
        return;
    }

    auto value = m_keyfile.getString(sectionName, fieldName);
    try {
        destination = convertStringToAddress(value);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(fmt::format(
            "Network section '{0}' field '{1}' validation failed - {2}",
            sectionName, fieldName, e.what()));
    }
}

void AnswerFile::loadNetwork(
    const std::string& networkSection, AFNetwork& network)
{
    LOG_TRACE("Loading network section {}", networkSection);

    network.con_interface = m_keyfile.getString(networkSection, "interface");
    convertNetworkAddressAndValidate(
        networkSection, "ip_address", network.con_ip_addr);
    network.con_mac_addr = m_keyfile.getString(networkSection, "mac_address", "");
    convertNetworkAddressAndValidate(
        networkSection, "subnet_mask", network.subnet_mask);
    network.domain_name = m_keyfile.getString(networkSection, "domain_name", "");
    convertNetworkAddressAndValidate(
        networkSection, "gateway", network.gateway);

    if (auto opt = m_keyfile.getStringOpt(networkSection, "nameservers")) {
        std::vector<std::string> nameservers;
        boost::split(nameservers, opt.value(),
            boost::is_any_of(", "), boost::token_compress_on);

        network.nameservers = nameservers;
    }
}

void AnswerFile::loadExternalNetwork()
{
    loadNetwork("network_external", external);
}

void AnswerFile::loadManagementNetwork()
{
    loadNetwork("network_management", management);
}

void AnswerFile::loadServiceNetwork()
{
    if (!m_keyfile.hasGroup("network_service"))
        return;

    loadNetwork("network_service", service);
}

void AnswerFile::loadApplicationNetwork()
{
    if (!m_keyfile.hasGroup("network_application"))
        return;

    loadNetwork("network_application", application);
}

void AnswerFile::loadInformation()
{
    information.cluster_name
        = m_keyfile.getString("information", "cluster_name");
    information.company_name
        = m_keyfile.getString("information", "company_name");
    information.administrator_email
        = m_keyfile.getString("information", "administrator_email");
}

void AnswerFile::loadTimeSettings()
{
    time.timezone = m_keyfile.getString("time", "timezone");
    time.timeserver = m_keyfile.getString("time", "timeserver");
    time.locale = m_keyfile.getString("time", "locale");
}

void AnswerFile::loadHostnameSettings()
{
    hostname.hostname = m_keyfile.getString("hostname", "hostname");
    hostname.domain_name = m_keyfile.getString("hostname", "domain_name");
}

void AnswerFile::loadSystemSettings()
{
    system.disk_image = m_keyfile.getString("system", "disk_image");
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();

    // Verify supported distros
    auto afDistro = m_keyfile.getString("system", "distro");
    if (const auto& formatDistro
        = cloyster::utils::enums::ofStringOpt<OS::Distro>(
            afDistro, cloyster::utils::enums::Case::Insensitive)) {
        system.distro = formatDistro.value();
    } else {
        if (opts->dryRun) {
            return;
        }
        throw std::runtime_error(
            fmt::format("Unsupported distro: {}", afDistro));
    }

    system.version = m_keyfile.getString("system", "version");
    system.kernel = m_keyfile.getString("system", "kernel");
}

AFNode AnswerFile::loadNode(const std::string& section)
{
    AFNode node;
    LOG_DEBUG("Loading node {}", section);

    if (section == "node") {
        node.prefix = m_keyfile.getString(section, "prefix");
        node.padding = m_keyfile.getString(section, "padding");
    } else {
        node.mac_address = m_keyfile.getString(section, "mac_address");
    }

    if (m_keyfile.getStringOpt(section, "node_ip")) {
        try {
            node.start_ip = convertStringToAddress(
                m_keyfile.getString(section, "node_ip"));
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(
                fmt::format("Section '{}' field 'node_ip' "
                            "validation failed - {}",
                    section, e.what()));
        }
    }

    // Initialize with empty strings if the values are not found, the remaining
    // of the code is assuming that
    node.hostname = m_keyfile.getString(section, "hostname", "");
    node.root_password = m_keyfile.getString(section, "node_root_password", "");
    node.sockets = m_keyfile.getString(section, "sockets", "");
    node.cores_per_socket = m_keyfile.getString(section, "cores_per_socket", "");
    node.threads_per_core = m_keyfile.getString(section, "threads_per_core", "");
    node.bmc_address = m_keyfile.getString(section, "bmc_address", "");
    node.bmc_username = m_keyfile.getString(section, "bmc_username", "");
    node.bmc_password = m_keyfile.getString(section, "bmc_password", "");
    node.bmc_serialport = m_keyfile.getString(section, "bmc_serialport", "");
    node.bmc_serialspeed = m_keyfile.getString(section, "bmc_serialspeed", "");
    LOG_DEBUG("Node loaded {}", section);

    return node;
}

void AnswerFile::loadNodes()
{
    const AFNode generic = loadNode("node");
    nodes.generic = generic;

    /**
     * Some nodes will be out of order.
     * For example, the first node (node.1) will be commented out, but the
     * second node (node.2) will exist; the third (node.3) will be commented,
     * but the fourth (node.4) will exist
     *
     * The only rule we have is that the node must start at 0.
     *
     * For user convenience, we will accept files with this configuration
     */

    LOG_INFO("Enumerating nodes in the file");
    auto nodelist = m_keyfile.listAllPrefixedEntries("node.");

    LOG_INFO("Found {} possible nodes", nodelist.size());

    auto node_counter = [](std::string_view node) {
        auto num = node.substr(node.find_first_of('.') + 1);
        return boost::lexical_cast<std::size_t>(num);
    };

    auto is_node_number = [&node_counter](std::string_view node) {
        try {
            return node_counter(node) > 0;
        } catch (boost::bad_lexical_cast&) {
            return false;
        }
    };

    for (const auto& nodeSection :
        nodelist | std::views::filter(is_node_number)) {
        auto nodeCounter = node_counter(nodeSection);

        AFNode newNode = loadNode(nodeSection);

        if (newNode.hostname->empty()) {
            LOG_DEBUG("Node configured {}", newNode.hostname.value());
            if (generic.prefix->empty()) {
                throw std::invalid_argument(
                    fmt::format("Section node.{} must have a 'hostname' key or "
                                "you must inform a generic 'prefix' value",
                        nodeCounter));
            } else if (generic.padding->empty()) {
                throw std::invalid_argument(fmt::format(
                    "Section node.{} must have a 'hostname' key or you must "
                    "inform a generic 'padding' value",
                    nodeCounter));
            } else {
                newNode.hostname = fmt::format(fmt::runtime("{}{:0>{}}"),
                    generic.prefix.value(), nodeCounter,
                    stoi(generic.padding.value()));
            }
        }

        try {
            newNode = validateNode(newNode);
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(
                fmt::format("Section node.{} validation failed - {}",
                    nodeCounter, e.what()));
        }

        nodes.nodes.emplace_back(newNode);
    }
}

AFNode AnswerFile::validateNode(AFNode node)
{
    validateAttribute(
        "node", "node_ip", node.start_ip, nodes.generic->start_ip);
    validateAttribute("node", "node_root_password", node.root_password,
        nodes.generic->root_password);
    validateAttribute("node", "sockets", node.sockets, nodes.generic->sockets);
    validateAttribute("node", "cores_per_socket", node.cores_per_socket,
        nodes.generic->cores_per_socket);
    validateAttribute("node", "threads_per_core", node.threads_per_core,
        nodes.generic->threads_per_core);
    validateAttribute(
        "node", "bmc_address", node.bmc_address, nodes.generic->bmc_address);
    validateAttribute(
        "node", "bmc_username", node.bmc_username, nodes.generic->bmc_username);
    validateAttribute(
        "node", "bmc_password", node.bmc_password, nodes.generic->bmc_password);
    validateAttribute("node", "bmc_serialport", node.bmc_serialport,
        nodes.generic->bmc_serialport);
    validateAttribute("node", "bmc_serialspeed", node.bmc_serialspeed,
        nodes.generic->bmc_serialspeed);

    return node;
}

bool AnswerFile::checkEnabled(const std::string& section)
{
    return m_keyfile.getStringOpt(section, "enabled")
        && m_keyfile.getString(section, "enabled") == "1";
}

void AnswerFile::loadPostfix()
{
    if (!m_keyfile.hasGroup("postfix"))
        return;

    LOG_TRACE("Postfix enabled");

    postfix.enabled = true;

    boost::split(postfix.destination,
        m_keyfile.getString("postfix", "destination"), boost::is_any_of(", "),
        boost::token_compress_on);

    auto castProfile = cloyster::utils::enums::ofStringOpt<Postfix::Profile>(
        m_keyfile.getString("postfix", "profile"),
        cloyster::utils::enums::Case::Insensitive);

    if (castProfile.has_value())
        postfix.profile = castProfile.value();
    else {
        throw std::runtime_error(fmt::format("Invalid Postfix profile"));
    }

    AFPostfix::SMTP smtp;
    AFPostfix::SASL sasl;

    switch (postfix.profile) {
        case Postfix::Profile::Local:
            break;
        case Postfix::Profile::Relay:
            smtp.server = m_keyfile.getString("postfix.relay", "server");
            smtp.port = std::stoi(m_keyfile.getString("postfix.relay", "port"));
            postfix.smtp = smtp;
            break;
        case Postfix::Profile::SASL:
            smtp.server = m_keyfile.getString("postfix.sasl", "server");
            smtp.port = std::stoi(m_keyfile.getString("postfix.sasl", "port"));
            sasl.username = m_keyfile.getString("postfix.sasl", "username");
            sasl.password = m_keyfile.getString("postfix.sasl", "password");
            smtp.sasl = sasl;
            postfix.smtp = smtp;
            break;
    }

    postfix.cert_file = m_keyfile.getString("postfix", "smtpd_tls_cert_file");
    postfix.key_file = m_keyfile.getString("postfix", "smtpd_tls_key_file");
}

void AnswerFile::loadOFED()
{
    auto kind = m_keyfile.getString("ofed", "kind");
    if (kind != "") {
        ofed.enabled = true;
        ofed.kind = kind;
        auto afVersion = m_keyfile.getString("ofed", "version");
        if (afVersion != "") {
            ofed.version = afVersion;
        } else {
            ofed.version = "latest"; // use as default
        }

        LOG_DEBUG("OFED enabled, {} {}", ofed.kind, ofed.version.value())
    }
}

};
