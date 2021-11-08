#include "headnode.h"

#include <iostream>
#include <fstream>
#include <algorithm> /* std::remove */
#include <map> /* std::map */
#include <regex>

#include <sys/utsname.h>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

/* The constructor should discover everything we need from the machine that is
 * running the software. We always consider that the software runs for what will
 * become the cluster headnode.
 */
//Headnode::Headnode () = default;

Headnode::Headnode()
    : m_hostname(discoverHostname()) {}

/* We should refactor to boost::property_tree on both methods: fetchValue() and
 * setOS(); an those methods should really be on OS class and not here.
 */
std::string Headnode::fetchValue (const std::string& line) {
    std::string value;

    /* Get values from keys */
    size_t pos = line.find_first_of('=');
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

    return value;    
}

/* TODO: Throw exceptions on errors */
int Headnode::discoverOS () {
    struct utsname system {};

    uname(&system);

    if (std::string{system.machine} != "x86_64") {
        return -1;
    }

    this->m_os.arch = OS::Arch::x86_64;

    /* A map would be a better ideia:
     * std::map<std::string, Family> osFamily
     */
    if (std::string{system.sysname} == "Linux")
        this->m_os.family = OS::Family::Linux;
    if (std::string{system.sysname} == "Darwin")
        this->m_os.family = OS::Family::Darwin;

    /* Store kernel release in string format */
    this->m_os.kernel = std::string{system.release};

#ifdef _DEBUG_
    std::cout << "Architecture: " << (int)this->m_os.arch << std::endl;
    std::cout << "Family: " << (int)this->m_os.family << std::endl;
    std::cout << "Kernel Release: " << this->m_os.kernel << std::endl;
#endif

#ifdef _DUMMY_
    if (std::string{system.sysname} == "Darwin") {
        std::string filename = "chroot/etc/m_os-release";
#else
    if (std::string{system.sysname} == "Linux") {
        std::string filename = "/etc/os-release";
#endif
        std::ifstream file(filename);

        if (!file.is_open()) {
            perror(("Error while opening file " + filename).c_str());
            return -2;
        }

        /* Fetches OS information from /etc/m_os-release. The file is writen in a
         * key=value style.
         */
        std::string line;
        while (getline(file, line)) {

#if __cplusplus >= 202002L
            if (line.starts_with("PLATFORM_ID=")) {
#else
            if (boost::algorithm::starts_with(line, "PLATFORM_ID=")) {
#endif

                std::string parser = fetchValue(line);
                if (parser.substr(parser.find(':') + 1) == "el8")
                    this->m_os.platform = OS::Platform::el8;
            }

#if __cplusplus >= 202002L
            if (line.starts_with("ID=")) {
#else
            if (boost::algorithm::starts_with(line, "ID=")) {
#endif

                if (fetchValue(line) == "rhel")
                    this->m_os.distro = OS::Distro::RHEL;
                if (fetchValue(line) == "ol")
                    this->m_os.distro = OS::Distro::OL;
            }

#if __cplusplus >= 202002L
            if (line.starts_with("VERSION=")) {
#else
            if (boost::algorithm::starts_with(line, "VERSION=")) {
#endif

                std::string parser = fetchValue(line);

                this->m_os.majorVersion = stoi(
                    parser.substr(0, parser.find('.')));
                
                this->m_os.minorVersion = stoi(
                    parser.substr(parser.find('.') + 1));
            }
        }

        if (file.bad()) {
            perror(("Error while reading file " + filename).c_str());
            return -3;
        }

        file.close();
    }

#ifdef _DEBUG_
    std::cout << "Platform: " << (int)this->m_os.platform << std::endl;
    std::cout << "Distribution: " << (int)this->m_os.distro << std::endl;
    std::cout << "Major Version: " << this->m_os.majorVersion << std::endl;
    std::cout << "Minor Version: " << this->m_os.minorVersion << std::endl;
#endif

    return 0;
}

void Headnode::printOS () {
    std::cout << "Architecture: " << (int)this->m_os.arch << std::endl;
    std::cout << "Family: " << (int)this->m_os.family << std::endl;
    std::cout << "Kernel Release: " << this->m_os.kernel << std::endl;
    std::cout << "Platform: " << (int)this->m_os.platform << std::endl;
    std::cout << "Distribution: " << (int)this->m_os.distro << std::endl;
    std::cout << "Major Version: " << this->m_os.majorVersion << std::endl;
    std::cout << "Minor Version: " << this->m_os.minorVersion << std::endl;
}

/* Supported releases must be a constexpr defined elsewhere and not hardcoded as
 * a value on the code.
 * Return values should be errorcodes that match the failure and not being
 * written directly on the method. Also, they might need to be a bitmap or enum
 */
int Headnode::checkSupportedOS () {
    if (this->m_os.arch != OS::Arch::x86_64) {
#ifdef _DEBUG_
        std::cout << (int)this->m_os.arch 
            << " is not a supported architecture" << std::endl;
#endif
        return -1;
    }

    if (this->m_os.family != OS::Family::Linux) {
#ifdef _DEBUG_
        std::cout << (int)this->m_os.family 
            << " is not a supported operating system" << std::endl;
#endif
        return -2;
    }

    if (this->m_os.platform != OS::Platform::el8) {
#ifdef _DEBUG_
        std::cout << (int)this->m_os.platform 
            << " is not a supported Linux platform" << std::endl;
#endif
        return -3;
    }

    if ((this->m_os.distro != OS::Distro::RHEL) &&
        (this->m_os.distro != OS::Distro::OL)) {
#ifdef _DEBUG_
        std::cout << (int)this->m_os.distro 
            << " is not a supported Linux distribution" << std::endl;
#endif
        return -4;
    }

    if (this->m_os.majorVersion < 8) {
#ifdef _DEBUG_
        std::cout << "This software is only supported on EL8" << std::endl;
#endif
        return -5;
    }

    return 0;
}

const std::string &Headnode::getHostname() const {
    return m_hostname;
}

void Headnode::setHostname(const std::string &hostname) {
    if (hostname.size() > 63)
        throw;

#if __cplusplus >= 202002L
    if (hostname.starts_with('-') or hostname.ends_with('-'))
#else
    if (boost::algorithm::starts_with(hostname, '-') or
        boost::algorithm::ends_with(hostname, '-'));
#endif
        throw;

    /* Check if string has only digits */
    if (std::regex_match(hostname, std::regex("^[0-9]+$")))
        throw;
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(hostname, std::regex("^[A-Za-z0-9-]+$"))))
        throw;
    
    m_hostname = hostname;
}

const std::string Headnode::discoverHostname() {
    struct utsname system {};

    uname(&system);
    std::string_view hostname = system.nodename;
    m_hostname = hostname.substr(0, hostname.find('.'));

    return m_hostname;
}

const std::string &Headnode::getFQDN() const {
    return m_fqdn;
}

void Headnode::setFQDN(const std::string &fqdn) {
    if (fqdn.size() > 255)
        throw;

    m_fqdn = fqdn;
}

const OS &Headnode::getOS() const {
    return m_os;
}

void Headnode::setOS(const OS &os) {
    m_os = os;
}

//const std::unique_ptr<Connection>& Headnode::getConnection() const {
//    return m_externalConnection;
//}

const std::vector<Connection>& Headnode::getConnection() const {
    return m_externalConnection;
}

void Headnode::addConnection(const std::shared_ptr<Network>& network,
                             const std::string& interface,
                             const std::string& address) {
    m_externalConnection.emplace_back(network, interface, address);
}
