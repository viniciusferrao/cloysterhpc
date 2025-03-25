/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_OS_H_
#define CLOYSTERHPC_OS_H_

#include <cloysterhpc/const.h>
#include <cloysterhpc/services/package_manager.h>
#include <fmt/format.h>
#include <gsl/gsl-lite.hpp>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <string>
#include <variant>

namespace cloyster::models {
/**
 * @class OS
 * @brief A class representing an Operating System (OS).
 *
 * This class provides functionality to manage various attributes of an
 * operating system, including architecture, family, platform, distribution,
 * kernel version, and version number.
 *
 * Support for Darwin added for development reasons, not really supported.
 */
class OS {
public:
    OS(const OS&) = default;
    OS& operator=(const OS&) = default;
    OS(OS&&) = delete;
    OS& operator=(OS&&) = delete;
    ~OS() = default;

    /**
     * @enum Arch
     * @brief Enumeration representing different architectures of the OS.
     */
    enum class Arch { x86_64, ppc64le };

    /**
     * @enum Family
     * @brief Enumeration representing different families of the OS.
     */
    enum class Family { Linux, Darwin };

    /**
     * @enum Platform
     * @brief Enumeration representing different platforms of the OS.
     */
    enum class Platform { el8, el9, el10 };

    /**
     * @enum Distro
     * @brief Enumeration representing different distributions of the OS.
     */
    enum class Distro { RHEL, OL, Rocky, AlmaLinux };

    /**
     * @enum PackageManager
     * @brief What
     */
    enum class PackageType { RPM, DEB };

private:
    std::variant<std::monostate, Arch> m_arch;
    std::variant<std::monostate, Family> m_family;
    std::variant<std::monostate, Platform> m_platform;
    std::variant<std::monostate, Distro> m_distro;
    std::string m_kernel;
    unsigned m_majorVersion {};
    unsigned m_minorVersion {};

    std::shared_ptr<package_manager> m_packageManager;

    void setMajorVersion(unsigned int majorVersion);

    void setMinorVersion(unsigned int minorVersion);

    /**
     * @brief Extracts the value from a key-value pair string.
     *
     * @param line The key-value pair string.
     * @return The value extracted from the key-value pair string.
     */
    static std::string getValueFromKey(const std::string& line);

    std::shared_ptr<package_manager> factoryPackageManager(
        OS::Platform platform);

public:
    OS();

    [[nodiscard]] Arch getArch() const;
    void setArch(Arch arch);
    void setArch(std::string_view arch);

    [[nodiscard]] Family getFamily() const;
    void setFamily(Family family);
    void setFamily(std::string_view family);

    [[nodiscard]] Platform getPlatform() const;
    void setPlatform(Platform platform);
    void setPlatform(std::string_view platform);

    [[nodiscard]] Distro getDistro() const;
    [[nodiscard]] std::string getDistroString() const;
    void setDistro(Distro distro);
    void setDistro(std::string_view distro);

    [[nodiscard]] std::string_view getKernel() const;
    void setKernel(std::string_view kernel);

    [[nodiscard]] std::string getVersion() const;
    void setVersion(const std::string&);

    [[nodiscard]] unsigned int getMajorVersion() const;
    [[nodiscard]] unsigned int getMinorVersion() const;

    gsl::not_null<package_manager*> packageManager() const;

    [[nodiscard]] PackageType getPackageType() const;

    /**
     * @brief Prints the data of the OS.
     *
     * This method is available only in debug mode.
     */
    void printData() const;
};

}; // namespace cloyster::models
#endif // CLOYSTERHPC_OS_H_
