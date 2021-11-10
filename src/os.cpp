#include "os.h"

#include <fmt/format.h>

OS::Arch OS::getArch() const {
    return m_arch;
}

void OS::setArch(OS::Arch arch) {
    m_arch = arch;
}

OS::Family OS::getFamily() const {
    return m_family;
}

void OS::setFamily(OS::Family family) {
    m_family = family;
}

OS::Platform OS::getPlatform() const {
    return m_platform;
}

void OS::setPlatform(OS::Platform platform) {
    m_platform = platform;
}

OS::Distro OS::getDistro() const {
    return m_distro;
}

void OS::setDistro(OS::Distro distro) {
    m_distro = distro;
}

const std::string &OS::getKernel() const {
    return m_kernel;
}

void OS::setKernel(const std::string &kernel) {
    m_kernel = kernel;
}

unsigned int OS::getMajorVersion() const {
    return m_majorVersion;
}

void OS::setMajorVersion(unsigned int majorVersion) {
    m_majorVersion = majorVersion;
}

unsigned int OS::getMinorVersion() const {
    return m_minorVersion;
}

void OS::setMinorVersion(unsigned int minorVersion) {
    m_minorVersion = minorVersion;
}

std::string OS::getVersion() const {
    return fmt::format("{}.{}", m_majorVersion, m_minorVersion);
}
