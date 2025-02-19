/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/dnf.h>
#include <cloysterhpc/services/package_manager.h>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <variant>

#include <cloysterhpc/services/log.h>

#include <fstream>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <sys/utsname.h>

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace cloyster::models {

OS::OS()
{
    struct utsname system {};
    uname(&system);

    setArch(system.machine);
    setFamily(system.sysname);
    setKernel(system.release);

#ifdef __APPLE__
    if (true) {
#else
    if (getFamily() == OS::Family::Linux) {
#endif
        std::string filename = CHROOT "/etc/os-release";
        LOG_INFO("Opening {}", filename);
        std::ifstream file(filename);

        if (!file.is_open()) {
            LOG_ERROR("Error while opening file {}", filename);
            throw std::runtime_error(
                fmt::format("Error while opening file: {}", filename));
        }

        /* Fetches OS information from /etc/os-release. The file is writen in a
         * key=value style.
         */
        std::string line;
        while (std::getline(file, line)) {

            /* TODO: Refactor the next three conditions */
            if (line.starts_with("PLATFORM_ID=")) {
                LOG_DEBUG("Found platform (PLATFORM_ID=)");
                auto value = getValueFromKey(line);
                if (value.starts_with("platform:")) {
                    // Skip the 'platform:' prefix
                    constexpr auto platform = std::string_view("platform:");
                    setPlatform(value.substr(platform.size()));
                } else {
                    setPlatform(value);
                }
            }

            if (line.starts_with("ID=")) {
                LOG_DEBUG("Found distro (ID=)");
                setDistro(getValueFromKey(line));
            }

            if (line.starts_with("VERSION=")) {
                LOG_DEBUG("Found version (VERSION=)");
                setVersion(getValueFromKey(line));
            }
        }

        if (file.bad()) {
            LOG_ERROR("Error while reading file {}", filename);
            throw std::runtime_error(
                fmt::format("Error while reading file: {}", filename));
        }
    }

    factoryPackageManager(getPlatform());
}

// OS::OS(OS::Arch arch, OS::Family family, OS::Platform platform,
//     OS::Distro distro, std::string_view kernel, unsigned majorVersion,
//     unsigned minorVersion)
//     : m_arch(arch)
//     , m_family(family)
//     , m_platform(platform)
//     , m_distro(distro)
//     , m_packageManager(factoryPackageManager(platform))
// {
//     setKernel(kernel);
//     setMajorVersion(majorVersion);
//     setMinorVersion(minorVersion);
// }
//
OS::Arch OS::getArch() const { return std::get<OS::Arch>(m_arch); }

void OS::setArch(Arch arch) { m_arch = arch; }

void OS::setArch(std::string_view arch)
{
    if (arch != "x86_64") {
        throw std::runtime_error(
            fmt::format("Unsupported architecture: {}", arch));
    }

    setArch(OS::Arch::x86_64);
}

OS::Family OS::getFamily() const { return std::get<OS::Family>(m_family); }

void OS::setFamily(Family family) { m_family = family; }

void OS::setFamily(std::string_view family)
{
    if (const auto& rv = magic_enum::enum_cast<Family>(family))
        setFamily(rv.value());
    else
        throw std::runtime_error(fmt::format("Unsupported OS: {}", family));
}

OS::Platform OS::getPlatform() const
{
    return std::get<OS::Platform>(m_platform);
}

void OS::setPlatform(OS::Platform platform) { m_platform = platform; }

void OS::setPlatform(std::string_view platform)
{
    std::string lowercasePlatform(platform);
    std::transform(lowercasePlatform.begin(), lowercasePlatform.end(),
        lowercasePlatform.begin(), ::tolower);

    for (const auto& enumValue : magic_enum::enum_values<Platform>()) {
        if (lowercasePlatform == magic_enum::enum_name(enumValue)) {
            setPlatform(enumValue);
            return;
        }
    }

    throw std::runtime_error(fmt::format("Unsupported Platform: {}", platform));
}

OS::Distro OS::getDistro() const
{
    LOG_ASSERT(!std::holds_alternative<std::monostate>(m_distro),
        "m_distro is uninitialized");
    return std::get<OS::Distro>(m_distro);
}

OS::PackageType OS::getPackageType() const
{
    switch (getDistro()) {
        case Distro::RHEL:
        case Distro::OL:
        case Distro::Rocky:
        case Distro::AlmaLinux:
            return PackageType::RPM;
        default:
            throw std::runtime_error("Unknonw distro type");
    };
}

void OS::setDistro(OS::Distro distro) { m_distro = distro; }

void OS::setDistro(std::string_view distro)
{
    // This code block is left for future reference, if an insensitive
    // comparison in magic_enum would be implemented it may easily replace the
    // lambda block. Reference: https://github.com/Neargye/magic_enum/pull/139

#if 1
    if (const auto& rv
        = magic_enum::enum_cast<Distro>(distro, magic_enum::case_insensitive))
#else
    if (const auto &rv
        = magic_enum::enum_cast<Distro>(distro, [](char lhs, char rhs) {
              return std::tolower(lhs) == std::tolower(rhs);
          }))
#endif
        setDistro(rv.value());
    else
        throw std::runtime_error(
            fmt::format("Unsupported Distribution: {}", distro));
}

std::string_view OS::getKernel() const { return m_kernel; }

void OS::setKernel(std::string_view kernel) { m_kernel = kernel; }

unsigned int OS::getMajorVersion() const { return m_majorVersion; }

void OS::setMajorVersion(unsigned int majorVersion)
{
    if (majorVersion < 8)
        throw std::runtime_error(
            "Unsupported release: Major version must be 8 or greater.");

    m_majorVersion = majorVersion;
}

unsigned int OS::getMinorVersion() const { return m_minorVersion; }

void OS::setMinorVersion(unsigned int minorVersion)
{
    m_minorVersion = minorVersion;
}

std::string OS::getVersion() const
{
    return fmt::format("{}.{}", m_majorVersion, m_minorVersion);
}

void OS::setVersion(const std::string& version)
{
    setMajorVersion(static_cast<unsigned>(
        std::stoul(version.substr(0, version.find('.')))));

    // FIXME: Read the value from the ISO file intead of
    // expecting it to be explicit in answerfile.ini

    // We expect the system.version in the answerfile
    // to be in the format M.N, and abort if it is not valid
    if (version.find('.') == std::string::npos) {
        throw std::runtime_error(fmt::format(
            "Unexpected value for system.version (in answerfile.ini). "
            "Expected M.N format, e.g., 9.5. Value found instead: {}",
            version));
    }

    setMinorVersion(
        static_cast<unsigned>(stoul(version.substr(version.find('.') + 1))));
}

/* We should refactor to boost::property_tree on both methods:
 * fetchValueFromKey() and setOS(); an those methods should really be on OS
 * class and not here.
 */

std::string OS::getValueFromKey(const std::string& line)
{
    std::string value;

    /* Get values from keys */
    std::size_t pos = line.find_first_of('=');
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

    return value;
}

std::shared_ptr<package_manager> OS::factoryPackageManager(
    OS::Platform platform)
{
    for (const auto& supportedPlatform : magic_enum::enum_values<Platform>()) {
        if (platform == supportedPlatform) {
            m_packageManager = std::make_shared<dnf>();
            return m_packageManager;
        }
    }

    throw std::runtime_error(fmt::format(
        "Unsupported OS platform: {}", magic_enum::enum_name(platform)));
}

gsl::not_null<package_manager*> OS::packageManager() const
{
    return m_packageManager.get();
}

void OS::printData() const
{
#ifndef NDEBUG
    LOG_DEBUG("Architecture: {}", magic_enum::enum_name(std::get<Arch>(m_arch)))
    LOG_DEBUG("Family: {}", magic_enum::enum_name(std::get<Family>(m_family)))
    LOG_DEBUG("Kernel Release: {}", m_kernel)
    LOG_DEBUG(
        "Platform: {}", magic_enum::enum_name(std::get<Platform>(m_platform)))
    LOG_DEBUG(
        "Distribution: {}", magic_enum::enum_name(std::get<Distro>(m_distro)))
    LOG_DEBUG("Major Version: {}", m_majorVersion)
    LOG_DEBUG("Minor Version: {}", m_minorVersion)
#endif
}
};
