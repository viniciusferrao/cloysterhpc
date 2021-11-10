#ifndef OS_H
#define OS_H

#include <string>

/* Darwin added for development reasons, not really supported. */
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
    unsigned m_majorVersion;
    unsigned m_minorVersion;

private:
    unsigned int getMajorVersion() const;
    void setMajorVersion(unsigned int majorVersion);

    unsigned int getMinorVersion() const;
    void setMinorVersion(unsigned int minorVersion);

public:
    Arch getArch() const;
    void setArch(Arch arch);

    Family getFamily() const;
    void setFamily(Family family);

    Platform getPlatform() const;
    void setPlatform(Platform platform);

    Distro getDistro() const;
    void setDistro(Distro distro);

    const std::string &getKernel() const;
    void setKernel(const std::string &kernel);

    std::string getVersion() const;
};

#endif /* OS_H */
