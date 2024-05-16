/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_OS_H_
#define CLOYSTERHPC_OS_H_

#include <cloysterhpc/const.h>
#include <string>

// Darwin added for development reasons, not really supported.
/**
 * @class OS
 * @brief A class representing an Operating System (OS).
 *
 * This class provides functionality to manage various attributes of an
 * operating system, including architecture, family, platform, distribution,
 * kernel version, and version number.
 */
class OS {
public:
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
    enum class Platform { el8, el9 };

    /**
     * @enum Distro
     * @brief Enumeration representing different distributions of the OS.
     */
    enum class Distro { RHEL, OL, Rocky, AlmaLinux };

private:
    Arch m_arch;
    Family m_family;
    Platform m_platform;
    Distro m_distro;
    std::string m_kernel;
    unsigned m_majorVersion {};
    unsigned m_minorVersion {};

private:
    void setMajorVersion(unsigned int majorVersion);

    void setMinorVersion(unsigned int minorVersion);

    /**
     * @brief Extracts the value from a key-value pair string.
     *
     * @param line The key-value pair string.
     * @return The value extracted from the key-value pair string.
     */
    std::string getValueFromKey(const std::string& line);

public:
    OS();

    /**
     * @brief Constructs an OS object with the specified attributes.
     *
     * @param arch The architecture of the OS.
     * @param family The family of the OS.
     * @param platform The platform of the OS.
     * @param distro The distribution of the OS.
     * @param kernel The kernel version of the OS.
     * @param majorVersion The major version number of the OS.
     * @param minorVersion The minor version number of the OS.
     */
    OS(OS::Arch arch, OS::Family family, OS::Platform platform,
        OS::Distro distro, std::string_view kernel, unsigned majorVersion,
        unsigned minorVersion);

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
    void setDistro(Distro distro);
    void setDistro(std::string_view distro);

    [[nodiscard]] std::string_view getKernel() const;
    void setKernel(std::string_view kernel);

    [[nodiscard]] std::string getVersion() const;
    void setVersion(const std::string&);

    [[nodiscard]] unsigned int getMajorVersion() const;
    [[nodiscard]] unsigned int getMinorVersion() const;

#ifndef NDEBUG
    /**
     * @brief Prints the data of the OS.
     *
     * This method is available only in debug mode.
     */
    void printData() const;
#endif
};

#endif // CLOYSTERHPC_OS_H_
