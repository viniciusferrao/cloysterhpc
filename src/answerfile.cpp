/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/answerfile.h"
#include "cloysterhpc/services/log.h"
#include "cloysterhpc/tools/nvhpc.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <magic_enum.hpp>

AnswerFile::AnswerFile(const std::filesystem::path& path)
    : m_path(path)
{
    loadFile(m_path);
}

AnswerFile::AnswerFile() {};

void AnswerFile::loadFile(const std::filesystem::path& path)
{
    m_path = path;
    m_ini.loadFile(m_path);
#ifndef BUILD_TESTING
    loadOptions();
#endif
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
    loadTools();
    loadPostfix();
}

address AnswerFile::convertStringToAddress(const std::string& addr)
{
    try {
        return boost::asio::ip::make_address(addr);
    } catch (boost::system::system_error& e) {
        throw std::invalid_argument("Invalid address");
    }
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
    if (!m_ini.exists(sectionName, fieldName))
        return;

    auto value = m_ini.getValue(sectionName, fieldName, isOptional);
    try {
        destination = convertStringToAddress(value);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(fmt::format(
            "Network section '{0}' field '{1}' validation failed - {2}",
            sectionName, fieldName, e.what()));
    }
}

template <typename NetworkType>
void AnswerFile::loadNetwork(const std::string& networkSection,
    NetworkType& network, bool optionalNameservers)
{
    network.con_interface = m_ini.getValue(networkSection, "interface", false);
    convertNetworkAddressAndValidate(
        networkSection, "ip_address", network.con_ip_addr);
    network.con_mac_addr = m_ini.getValue(networkSection, "mac_address");
    convertNetworkAddressAndValidate(
        networkSection, "subnet_mask", network.subnet_mask);
    network.domain_name = m_ini.getValue(networkSection, "domain_name");
    convertNetworkAddressAndValidate(
        networkSection, "gateway", network.gateway);

    if (m_ini.exists(networkSection, "nameservers")) {
        std::vector<std::string> nameservers;
        boost::split(nameservers,
            m_ini.getValue(networkSection, "nameservers", optionalNameservers),
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
    if (!m_ini.exists("network_service"))
        return;

    loadNetwork("network_service", service);
}

void AnswerFile::loadApplicationNetwork()
{
    if (!m_ini.exists("network_application"))
        return;

    loadNetwork("network_application", application, false);
}

void AnswerFile::loadInformation()
{
    information.cluster_name
        = m_ini.getValue("information", "cluster_name", false, false);
    information.company_name
        = m_ini.getValue("information", "company_name", false, false);
    information.administrator_email
        = m_ini.getValue("information", "administrator_email", false, false);
}

void AnswerFile::loadTimeSettings()
{
    time.timezone = m_ini.getValue("time", "timezone", false, false);
    time.timeserver = m_ini.getValue("time", "timeserver", false, false);
    time.locale = m_ini.getValue("time", "locale", false, false);
}

void AnswerFile::loadHostnameSettings()
{
    hostname.hostname = m_ini.getValue("hostname", "hostname", false, false);
    hostname.domain_name
        = m_ini.getValue("hostname", "domain_name", false, false);
}

void AnswerFile::loadSystemSettings()
{
    system.disk_image = m_ini.getValue("system", "disk_image", false, false);

    // Verify supported distros
    auto afDistro = m_ini.getValue("system", "distro", false, false);
    if (const auto& formatDistro = magic_enum::enum_cast<OS::Distro>(
            afDistro, magic_enum::case_insensitive)) {
        system.distro = formatDistro.value();
    } else {
        throw std::runtime_error(
            fmt::format("Unsupported distro: {}", afDistro));
    }

    system.version = m_ini.getValue("system", "version", false, false);
    system.kernel = m_ini.getValue("system", "kernel", false, false);
}

AnswerFile::AFNode AnswerFile::loadNode(const std::string& section)
{
    AFNode node;

    if (section == "node") {
        node.prefix = m_ini.getValue(section, "prefix");
        node.padding = m_ini.getValue(section, "padding");
    } else {
        node.mac_address = m_ini.getValue(section, "mac_address", false);
    }

    if (m_ini.exists(section, "node_ip")) {
        try {
            node.start_ip
                = convertStringToAddress(m_ini.getValue(section, "node_ip"));
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(
                fmt::format("Section '{}' field 'node_ip' "
                            "validation failed - {}",
                    section, e.what()));
        }
    }

    node.hostname = m_ini.getValue(section, "hostname");
    node.root_password = m_ini.getValue(section, "node_root_password");
    node.sockets = m_ini.getValue(section, "sockets");
    node.cores_per_socket = m_ini.getValue(section, "cores_per_socket");
    node.threads_per_core = m_ini.getValue(section, "threads_per_core");
    node.bmc_address = m_ini.getValue(section, "bmc_address");
    node.bmc_username = m_ini.getValue(section, "bmc_username");
    node.bmc_password = m_ini.getValue(section, "bmc_password");
    node.bmc_serialport = m_ini.getValue(section, "bmc_serialport");
    node.bmc_serialspeed = m_ini.getValue(section, "bmc_serialspeed");

    return node;
}

void AnswerFile::loadNodes()
{
    const AFNode generic = loadNode("node");
    nodes.generic = generic;

    int nodeCounter = 1;
    while (true) {
        std::string nodeSection = fmt::format("node.{}", nodeCounter);
        if (!m_ini.exists(nodeSection)) {
            break;
        }

        LOG_TRACE("Configure {}", nodeSection)
        AFNode newNode = loadNode(nodeSection);

        if (newNode.hostname->empty()) {
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
            } else
                newNode.hostname = fmt::format(fmt::runtime("{}{:0>{}}"),
                    generic.prefix.value(), nodeCounter,
                    stoi(generic.padding.value()));
        }

        try {
            newNode = validateNode(newNode);
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(
                fmt::format("Section node.{} validation failed - {}",
                    nodeCounter, e.what()));
        }

        nodes.nodes.emplace_back(newNode);
        nodeCounter++;
    }
}

AnswerFile::AFNode AnswerFile::validateNode(AnswerFile::AFNode node)
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
    return m_ini.exists(section, "enabled")
        && m_ini.getValue(section, "enabled") == "1";
}

void AnswerFile::loadTools() { loadNVHPC(); }

void AnswerFile::loadNVHPC()
{
    if (!checkEnabled("nvhpc")) {
        return;
    }

    m_tools.emplace_back(std::make_shared<NVhpc>());
}

std::vector<std::shared_ptr<ITool>> AnswerFile::getTools() { return m_tools; }

void AnswerFile::loadPostfix()
{
    if (!m_ini.exists("postfix"))
        return;

    LOG_TRACE("Postfix enabled");

    postfix.enabled = true;

    boost::split(postfix.destination,
        m_ini.getValue("postfix", "destination", false), boost::is_any_of(", "),
        boost::token_compress_on);

    auto castProfile = magic_enum::enum_cast<Postfix::Profile>(
        m_ini.getValue("postfix", "profile", false),
        magic_enum::case_insensitive);

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
            smtp.server = m_ini.getValue("postfix.relay", "server", false);
            smtp.port
                = std::stoi(m_ini.getValue("postfix.relay", "port", false));
            postfix.smtp = smtp;
            break;
        case Postfix::Profile::SASL:
            smtp.server = m_ini.getValue("postfix.sasl", "server", false);
            smtp.port
                = std::stoi(m_ini.getValue("postfix.sasl", "port", false));
            sasl.username = m_ini.getValue("postfix.sasl", "username", false);
            sasl.password = m_ini.getValue("postfix.sasl", "password", false);
            smtp.sasl = sasl;
            postfix.smtp = smtp;
            break;
    }

    postfix.cert_file = m_ini.getValue("postfix", "smtpd_tls_cert_file", false);
    postfix.key_file = m_ini.getValue("postfix", "smtpd_tls_key_file", false);
}

#ifdef BUILD_TESTING
#include <cloysterhpc/tests.h>

TEST_SUITE("Test Answerfile Methods")
{
    TEST_CASE("Load Methods")
    {
        AnswerFile correct { tests::sampleDirectory
            / "answerfile/correct.answerfile.ini" };
        AnswerFile wrong { tests::sampleDirectory
            / "answerfile/wrong.answerfile.ini" };

        SUBCASE("Networks")
        {

            CHECK_NOTHROW(correct.loadExternalNetwork());
            CHECK_THROWS(wrong.loadExternalNetwork());

            CHECK_NOTHROW(correct.loadManagementNetwork());
            CHECK_THROWS(wrong.loadManagementNetwork());

            CHECK_NOTHROW(correct.loadApplicationNetwork());
            CHECK_THROWS(wrong.loadApplicationNetwork());
        }

        SUBCASE("Information")
        {
            CHECK_NOTHROW(correct.loadInformation());
            CHECK_THROWS(wrong.loadInformation());
        }

        SUBCASE("Time")
        {
            CHECK_NOTHROW(correct.loadTimeSettings());
            CHECK_THROWS(wrong.loadTimeSettings());
        }

        SUBCASE("Hostname")
        {
            CHECK_NOTHROW(correct.loadHostnameSettings());
            CHECK_THROWS(wrong.loadHostnameSettings());
        }

        SUBCASE("System")
        {
            CHECK_NOTHROW(correct.loadSystemSettings());
            CHECK_THROWS(wrong.loadSystemSettings());
        }

        SUBCASE("Nodes")
        {
            CHECK_NOTHROW(correct.loadNodes());
            CHECK_THROWS(wrong.loadNodes());
        }
    }
}
#endif
