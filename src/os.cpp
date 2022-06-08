/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "os.h"
#include "include/magic_enum.hpp"

#ifndef NDEBUG
#include "services/log.h"
#endif

#include <string>
#include <fstream>

#include <fmt/format.h>
#include <sys/utsname.h>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

OS::OS() {
    struct utsname system{};
    uname(&system);

    setArch(system.machine);
    setFamily(system.sysname);
    setKernel(system.release);

#ifdef __APPLE__
    if (true) {
#else
    if (getFamily() == OS::Family::Linux) {
#endif
        std::string filename = CHROOT"/etc/os-release";
        std::ifstream file(filename);

        if (!file.is_open()) {
            perror(("Error while opening file " + filename).c_str());
            throw; /* Error opening file */
        }

        /* Fetches OS information from /etc/os-release. The file is writen in a
         * key=value style.
         */
        std::string line;
        while (std::getline(file, line)) {

            /* TODO: Refactor the next three conditions */
#if __cpp_lib_starts_ends_with >= 201711L
            if (line.starts_with("PLATFORM_ID=")) {
#else
                if (boost::algorithm::starts_with(line, "PLATFORM_ID=")) {
#endif
                setPlatform(getValueFromKey(line));
            }

#if __cpp_lib_starts_ends_with >= 201711L
            if (line.starts_with("ID=")) {
#else
                if (boost::algorithm::starts_with(line, "ID=")) {
#endif
                setDistro(getValueFromKey(line));
            }

#if __cpp_lib_starts_ends_with >= 201711L
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

OS::OS(OS::Arch arch, OS::Family family, OS::Platform platform,
       OS::Distro distro, std::string_view kernel,
       unsigned majorVersion, unsigned minorVersion)
       : m_arch(arch)
       , m_family(family)
       , m_platform(platform)
       , m_distro(distro)
{
    setKernel(kernel);
    setMajorVersion(majorVersion);
    setMinorVersion(minorVersion);
}

OS::Arch OS::getArch() const {
    return m_arch;
}

void OS::setArch(Arch arch) {
    m_arch = arch;
}

void OS::setArch(std::string_view arch) {
    if (arch != "x86_64") {
        throw std::runtime_error("Unsupported architecture");
    }

    setArch(OS::Arch::x86_64);
}

OS::Family OS::getFamily() const {
    return m_family;
}

void OS::setFamily(Family family) {
    m_family = family;
}

void OS::setFamily(std::string_view family) {
    if (const auto& rv = magic_enum::enum_cast<Family>(family))
        setFamily(rv.value());
    else
        throw std::runtime_error(fmt::format("Unsupported OS: {}", family));
}

OS::Platform OS::getPlatform() const {
    return m_platform;
}

void OS::setPlatform(OS::Platform platform) {
    m_platform = platform;
}

void OS::setPlatform(std::string_view platform) {
    if (platform.substr(platform.find(':') + 1) == "el8")
        setPlatform(OS::Platform::el8);
    else
        throw std::runtime_error(
                fmt::format("Unsupported Platform: {}", platform));
}

OS::Distro OS::getDistro() const {
    return m_distro;
}

void OS::setDistro(OS::Distro distro) {
    m_distro = distro;
}

void OS::setDistro(std::string_view distro) {
// This code block is left for future reference, if an insensitive comparison in
// magic_enum would be implemented it may easily replace the lambda block.
// Reference: https://github.com/Neargye/magic_enum/pull/139
#if 0
    if (const auto& rv = magic_enum::enum_cast<Distro>(distro, magic_enum::case_insensitive))
#endif
    if (const auto& rv = magic_enum::enum_cast<Distro>(distro, [](char lhs, char rhs) { return std::tolower(lhs) == std::tolower(rhs); }))
        setDistro(rv.value());
    else
        throw std::runtime_error(
                fmt::format("Unsupported Distribution: {}", distro));
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
    setMajorVersion(static_cast<unsigned>(std::stoul(
            version.substr(0, version.find('.')))));

    setMinorVersion(static_cast<unsigned>(stoul(
            version.substr(version.find('.') + 1))));
}

/* We should refactor to boost::property_tree on both methods:
 * fetchValueFromKey() and setOS(); an those methods should really be on OS
 * class and not here.
 */
std::string OS::getValueFromKey (const std::string& line) {
    std::string value;

    /* Get values from keys */
    std::size_t pos = line.find_first_of('=');
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(std::remove(value.begin(), value.end(), 
                            '"'), value.end());

    return value;
}

#ifndef NDEBUG
void OS::printData() const {
    LOG_DEBUG("Architecture: {}", magic_enum::enum_name(m_arch));
    LOG_DEBUG("Family: {}", magic_enum::enum_name(m_family));
    LOG_DEBUG("Kernel Release: {}", m_kernel);
    LOG_DEBUG("Platform: {}", magic_enum::enum_name(m_platform));
    LOG_DEBUG("Distribution: {}", magic_enum::enum_name(m_distro));
    LOG_DEBUG("Major Version: {}", m_majorVersion);
    LOG_DEBUG("Minor Version: {}", m_minorVersion);
}
#endif
