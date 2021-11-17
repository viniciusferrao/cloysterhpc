#include "os.h"

#ifdef _DEBUG_
#include <iostream>
#endif

#include <string>
#include <fstream>

#include <fmt/format.h>
#include <sys/utsname.h>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

OS::OS() {
    discover();
}

OS::Arch OS::getArch() const {
    return m_arch;
}

void OS::setArch(Arch arch) {
    m_arch = arch;
}

void OS::setArch(std::string_view arch) {
    if (arch != "x86_64") {
        throw; /* Unsupported architecture */
    }

    m_arch = OS::Arch::x86_64;
}

OS::Family OS::getFamily() const {
    return m_family;
}

void OS::setFamily(Family family) {
    m_family = family;
}

void OS::setFamily(std::string_view family) {
    /* An unordered_map may be a better implementation:
     * std::map<std::string, Family> osFamily
     */
    if (family == "Linux") {
        m_family = OS::Family::Linux;
        return;
    }

    if (family == "Darwin") {
        m_family = OS::Family::Darwin;
        return;
    }

    throw; /* Unsupported Family */
}

OS::Platform OS::getPlatform() const {
    return m_platform;
}

void OS::setPlatform(OS::Platform platform) {
    m_platform = platform;
}

void OS::setPlatform(std::string_view platform) {
    if (platform.substr(platform.find(':') + 1) == "el8") {
        m_platform = OS::Platform::el8;
        return;
    }

    throw; /* Unsupported Linux platform */
}

OS::Distro OS::getDistro() const {
    return m_distro;
}

void OS::setDistro(OS::Distro distro) {
    m_distro = distro;
}

void OS::setDistro(std::string_view distro) {
    if (distro == "rhel") {
        m_distro = OS::Distro::RHEL;
        return;
    }

    if (distro == "ol") {
        m_distro = OS::Distro::OL;
        return;
    }

    throw; /* Unsupported Linux distribution */
}

std::string_view OS::getKernel() const {
    return m_kernel;
}

void OS::setKernel(std::string_view kernel) {
    m_kernel = kernel;
}

unsigned int OS::getMajorVersion() const {
    return m_majorVersion;
}

void OS::setMajorVersion(unsigned int majorVersion) {
    if (majorVersion < 8)
        throw; /* Unsupported release */

    m_majorVersion = majorVersion;
}

unsigned int OS::getMinorVersion() const {
    return m_minorVersion;
}

void OS::setMinorVersion(unsigned int minorVersion) {
    if (minorVersion < 1)
        throw; /* Unsupported minor release */

    m_minorVersion = minorVersion;
}

std::string OS::getVersion() const {
    return fmt::format("{}.{}", m_majorVersion, m_minorVersion);
}

void OS::setVersion(const std::string& version) {
    setMajorVersion(stoi(
            version.substr(0, version.find('.'))));

    setMinorVersion(stoi(
            version.substr(version.find('.') + 1)));
}

/* We should refactor to boost::property_tree on both methods:
 * fetchValueFromKey() and setOS(); an those methods should really be on OS
 * class and not here.
 */
std::string OS::getValueFromKey (const std::string& line) {
    std::string value;

    /* Get values from keys */
    size_t pos = line.find_first_of('=');
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(std::remove(value.begin(), value.end(), 
                            '"'), value.end());

    return value;
}

/* TODO:
 *  - Throw exceptions on errors
 *  - Use setters and getters, moving the check logic to there
 */
void OS::discover () {
    struct utsname system {};
    uname(&system);

    setArch(system.machine);
    setFamily(system.sysname);
    setKernel(system.release);

#ifdef _DUMMY_
    if (true) {
        std::string filename = "chroot/etc/os-release";
#else
    if (getFamily() == OS::Family::Linux) {
        std::string filename = "/etc/os-release";
#endif
        std::ifstream file(filename);

        if (!file.is_open()) {
            perror(("Error while opening file " + filename).c_str());
            throw; /* Error opening file */
        }

        /* Fetches OS information from /etc/os-release. The file is writen in a
         * key=value style.
         */
        std::string line;
        while (getline(file, line)) {

            /* TODO: Refactor the next three conditions */
#if __cplusplus >= 202002L
            if (line.starts_with("PLATFORM_ID=")) {
#else
            if (boost::algorithm::starts_with(line, "PLATFORM_ID=")) {
#endif
                setPlatform(getValueFromKey(line));
            }

#if __cplusplus >= 202002L
            if (line.starts_with("ID=")) {
#else
            if (boost::algorithm::starts_with(line, "ID=")) {
#endif
                setDistro(getValueFromKey(line));
            }

#if __cplusplus >= 202002L
            if (line.starts_with("VERSION=")) {
#else
            if (boost::algorithm::starts_with(line, "VERSION=")) {
#endif
                setVersion(getValueFromKey(line));
            }
        }

        if (file.bad()) {
            perror(("Error while reading file " + filename).c_str());
            throw; /* Error while reading file */
        }
    }
}

#ifdef _DEBUG_
void OS::print() const {
    std::cout << "Architecture: " << (int)m_arch << std::endl;
    std::cout << "Family: " << (int)m_family << std::endl;
    std::cout << "Kernel Release: " << m_kernel << std::endl;
    std::cout << "Platform: " << (int)m_platform << std::endl;
    std::cout << "Distribution: " << (int)m_distro << std::endl;
    std::cout << "Major Version: " << m_majorVersion << std::endl;
    std::cout << "Minor Version: " << m_minorVersion << std::endl;
}
#endif
