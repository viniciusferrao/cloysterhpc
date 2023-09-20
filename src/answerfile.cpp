/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/answerfile.h"
#include "cloysterhpc/services/log.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <magic_enum.hpp>

AnswerFile::AnswerFile(const std::filesystem::path& path)
    : m_path(path)
{
    m_ini.loadFile(m_path);
    loadOptions();
}

void AnswerFile::loadOptions()
{
    LOG_TRACE("Read answerfile variables");

    loadExternalNetwork();
    loadManagementNetwork();
    loadApplicationNetwork();
    loadInformation();
    loadTimeSettings();
    loadHostnameSettings();
    loadSystemSettings();
    loadNodes();
    loadPostfix();
}

address AnswerFile::convertStringToAddress(const std::string& addr)
{
    try {
        return boost::asio::ip::make_address(addr);
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid address");
    }
}

void AnswerFile::loadExternalNetwork()
{
    external.con_interface
        = m_ini.getValue("network_external", "interface", false);

    if (m_ini.exists("network_external", "ip_address")) {
        try {
            external.con_ip_addr = convertStringToAddress(
                m_ini.getValue("network_external", "ip_address"));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section 'network_external' field 'ip_address' "
                            "validation failed - {}",
                    e.what()));
        }
    }

    external.con_mac_addr = m_ini.getValue("network_external", "mac_address");

    if (m_ini.exists("network_external", "subnet_mask")) {
        try {
            external.subnet_mask = convertStringToAddress(
                m_ini.getValue("network_external", "subnet_mask"));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section 'network_external' field 'subnet_mask' "
                            "validation failed - {}",
                    e.what()));
        }
    }

    external.domain_name = m_ini.getValue("network_external", "domain_name");

    if (m_ini.exists("network_external", "gateway")) {
        try {
            external.gateway = convertStringToAddress(
                m_ini.getValue("network_external", "gateway"));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section 'network_external' field 'gateway' "
                            "validation failed - {}",
                    e.what()));
        }
    }

    if (m_ini.exists("network_external", "nameservers")) {
        std::vector<std::string> nameservers;
        boost::split(nameservers,
            m_ini.getValue("network_external", "nameservers"),
            boost::is_any_of(", "), boost::token_compress_on);

        external.nameservers = nameservers;
    }
}

void AnswerFile::loadManagementNetwork()
{
    management.con_interface
        = m_ini.getValue("network_management", "interface", false);

    try {
        management.con_ip_addr = convertStringToAddress(
            m_ini.getValue("network_management", "ip_address", false));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            fmt::format("Section 'network_management' field 'ip_address' "
                        "validation failed - {}",
                e.what()));
    }

    management.con_mac_addr
        = m_ini.getValue("network_management", "mac_address");

    try {
        management.subnet_mask = convertStringToAddress(
            m_ini.getValue("network_management", "subnet_mask", false));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            fmt::format("Section 'network_management' field 'subnet_mask' "
                        "validation failed - {}",
                e.what()));
    }

    management.domain_name
        = m_ini.getValue("network_management", "domain_name", false);

    if (m_ini.exists("network_management", "gateway")) {
        try {
            management.gateway = convertStringToAddress(
                m_ini.getValue("network_management", "gateway"));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section 'network_management' field 'gateway' "
                            "validation failed - {}",
                    e.what()));
        }
    }

    if (m_ini.exists("network_management", "nameservers")) {
        std::vector<std::string> nameservers;
        boost::split(nameservers,
            m_ini.getValue("network_management", "nameservers"),
            boost::is_any_of(", "), boost::token_compress_on);

        management.nameservers = nameservers;
    }
}

void AnswerFile::loadApplicationNetwork()
{
    if (!m_ini.exists("network_application"))
        return;

    application.con_interface
        = m_ini.getValue("network_application", "interface", false);

    try {
        application.con_ip_addr = convertStringToAddress(
            m_ini.getValue("network_application", "ip_address", false));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            fmt::format("Section 'network_application' field 'ip_address' "
                        "validation failed - {}",
                e.what()));
    }

    application.con_mac_addr
        = m_ini.getValue("network_application", "mac_address", false);

    try {
        application.subnet_mask = convertStringToAddress(
            m_ini.getValue("network_application", "subnet_mask", false));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            fmt::format("Section 'network_application' field 'subnet_mask' "
                        "validation failed - {}",
                e.what()));
    }

    application.domain_name
        = m_ini.getValue("network_application", "domain_name", false);

    try {
        application.gateway = convertStringToAddress(
            m_ini.getValue("network_application", "gateway", false));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            fmt::format("Section 'network_application' field 'gateway' "
                        "validation failed - {}",
                e.what()));
    }

    std::vector<std::string> nameservers;
    boost::split(nameservers,
        m_ini.getValue("network_application", "nameservers", false),
        boost::is_any_of(", "), boost::token_compress_on);

    application.nameservers = nameservers;
}

void AnswerFile::loadInformation()
{
    information.cluster_name
        = m_ini.getValue("information", "cluster_name", false);
    information.company_name
        = m_ini.getValue("information", "company_name", false);
    information.administrator_email
        = m_ini.getValue("information", "administrator_email", false);
}

void AnswerFile::loadTimeSettings()
{
    time.timezone = m_ini.getValue("time", "timezone", false);
    time.timeserver = m_ini.getValue("time", "timeserver", false);
    time.locale = m_ini.getValue("time", "locale", false);
}

void AnswerFile::loadHostnameSettings()
{
    hostname.hostname = m_ini.getValue("hostname", "hostname", false);
    hostname.domain_name = m_ini.getValue("hostname", "domain_name", false);
}

void AnswerFile::loadSystemSettings()
{
    system.disk_image = m_ini.getValue("system", "disk_image", false);
    system.distro = m_ini.getValue("system", "distro", false);
    system.version = m_ini.getValue("system", "version", false);
    system.kernel = m_ini.getValue("system", "kernel", false);
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

    if (m_ini.exists(section, "node_start_ip")) {
        try {
            node.start_ip = convertStringToAddress(
                m_ini.getValue(section, "node_start_ip"));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section '{}' field 'node_start_ip' "
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

        LOG_TRACE("Configure {}", nodeSection);
        AFNode newNode = loadNode(nodeSection);

        if (newNode.hostname->empty()) {
            if (generic.prefix->empty()) {
                throw std::runtime_error(
                    fmt::format("Section node.{} must have a 'hostname' key or "
                                "you must inform a generic 'prefix' value",
                        nodeCounter));
            } else if (generic.padding->empty()) {
                throw std::runtime_error(fmt::format(
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
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                fmt::format("Section node.{} validation failed - {}",
                    nodeCounter, e.what()));
        }

        nodes.nodes.emplace_back(newNode);
        nodeCounter++;
    }
}

AnswerFile::AFNode AnswerFile::validateNode(AnswerFile::AFNode node)
{
    if (node.start_ip->is_unspecified()) {
        if (nodes.generic->start_ip->is_unspecified()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "node_start_ip"));
        }
        node.start_ip = nodes.generic->start_ip;
    }

    if (node.root_password->empty()) {
        if (nodes.generic->root_password->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "node_root_password"));
        }
        node.root_password = nodes.generic->root_password;
    }

    if (node.sockets->empty()) {
        if (nodes.generic->sockets->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "sockets"));
        }
        node.sockets = nodes.generic->sockets;
    }

    if (node.cores_per_socket->empty()) {
        if (nodes.generic->cores_per_socket->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "cores_per_socket"));
        }
        node.cores_per_socket = nodes.generic->cores_per_socket;
    }

    if (node.threads_per_core->empty()) {
        if (nodes.generic->threads_per_core->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "threads_per_core"));
        }
        node.threads_per_core = nodes.generic->threads_per_core;
    }

    if (node.bmc_address->empty()) {
        if (nodes.generic->bmc_address->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key", "bmc_address"));
        }
        node.bmc_address = nodes.generic->bmc_address;
    }

    if (node.bmc_username->empty()) {
        if (nodes.generic->bmc_username->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "bmc_username"));
        }
        node.bmc_username = nodes.generic->bmc_username;
    }

    if (node.bmc_password->empty()) {
        if (nodes.generic->bmc_password->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "bmc_password"));
        }
        node.bmc_password = nodes.generic->bmc_password;
    }

    if (node.bmc_serialport->empty()) {
        if (nodes.generic->bmc_serialport->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "bmc_serialport"));
        }
        node.bmc_serialport = nodes.generic->bmc_serialport;
    }

    if (node.bmc_serialspeed->empty()) {
        if (nodes.generic->bmc_serialspeed->empty()) {
            throw std::runtime_error(
                fmt::format("Node must have a \"{0}\" key or you must inform a "
                            "generic \"{0}\" value",
                    "bmc_serialspeed"));
        }
        node.bmc_serialspeed = nodes.generic->bmc_serialspeed;
    }

    return node;
}

void AnswerFile::loadPostfix() {
    if (!m_ini.exists("postfix"))
        return;

    LOG_TRACE("Postfix enabled");

    postfix.enabled = true;

    boost::split(postfix.destination,
        m_ini.getValue("postfix", "destination", false),
        boost::is_any_of(", "), boost::token_compress_on);

    auto castProfile = magic_enum::enum_cast<Postfix::Profile>(
        m_ini.getValue("postfix", "profile", false),
        magic_enum::case_insensitive);

    if (castProfile.has_value())
        postfix.profile = castProfile.value();
    else {
        throw std::runtime_error(
            fmt::format("Invalid Postfix profile"));
    }

    AFPostfix::SMTP smtp;
    AFPostfix::SASL sasl;

    switch(postfix.profile) {
        case Postfix::Profile::Local:
            break;
        case Postfix::Profile::Relay:
            smtp.server = m_ini.getValue("postfix.relay", "server", false);
            smtp.port = std::stoi(m_ini.getValue("postfix.relay", "port", false));
            postfix.smtp = smtp;
            break;
        case Postfix::Profile::SASL:
            smtp.server = m_ini.getValue("postfix.sasl", "server", false);
            smtp.port = std::stoi(m_ini.getValue("postfix.sasl", "port", false));
            sasl.username = m_ini.getValue("postfix.sasl", "username", false);
            sasl.password = m_ini.getValue("postfix.sasl", "password", false);
            smtp.sasl = sasl;
            postfix.smtp = smtp;
            break;
    }
}
