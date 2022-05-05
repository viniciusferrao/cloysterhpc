#ifndef OS_H
#define OS_H

#include <string>
#include "const.h"

// Darwin added for development reasons, not really supported.
class OS {
public:
    enum class Arch { x86_64, ppc64le };
    enum class Family { Linux, Darwin }; 
    enum class Platform { el8 };
    enum class Distro { RHEL, OL };

private:
    Arch m_arch;
    Family m_family;
    Platform m_platform;
    Distro m_distro;
    std::string m_kernel;
    unsigned m_majorVersion{};
    unsigned m_minorVersion{};

private:
    [[nodiscard]] unsigned int getMajorVersion() const;
    void setMajorVersion(unsigned int majorVersion);

    [[nodiscard]] unsigned int getMinorVersion() const;
    void setMinorVersion(unsigned int minorVersion);

    std::string getValueFromKey (const std::string& line);

public:
    OS();
    OS(OS::Arch arch, OS::Family family, OS::Platform platform,
       OS::Distro distro, std::string_view kernel,
       unsigned majorVersion, unsigned minorVersion);

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

#ifndef NDEBUG
    void printData() const;
#endif
};

#endif /* OS_H */
