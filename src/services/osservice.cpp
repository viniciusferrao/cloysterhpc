#include <cloysterhpc/functions.h>
#include <cloysterhpc/utils/string.h>
#include <cloysterhpc/services/osservice.h>
#include <stdexcept>

namespace cloyster::services {
using cloyster::functions::makeUniqueDerived;
using cloyster::Singleton;
using cloyster::services::IOSService;

class ELOSService final : public IOSService {
public:
    [[nodiscard]] std::string getKernelInstalled() const override
    {
        auto runner = Singleton<IRunner>::get();
        // Get the kernel version from the kernel package, order by BUILDTIME
        // since there may be multiple kernels installed (previous kernels)
        return runner->checkOutput(
            "bash -c \"rpm -q kernel --qf '%{VERSION}-%{RELEASE} "
            "%{BUILDTIME}\n' | sort -nrk 2 | head -1 | awk '{print $1}'\"")[0];
    }

    [[nodiscard]] std::string getKernelRunning() const override
    {
        return Singleton<IRunner>::get()->checkOutput("uname -r")[0];
    }

    [[nodiscard]] std::string getLocale() const override
    {
        // localectl status outputs a line like this System Locale:
        // LANG=en_US.utf8 The awk gets the en_US.utf8 part
        return Singleton<IRunner>::get()->checkOutput(
            R"(bash -c "localectl status | awk -F'=' '/System Locale: / {print $2}'")")
            [0];
    }

    [[nodiscard]] std::vector<std::string> getAvailableLocales() const override
    {
        auto runner = cloyster::Singleton<IRunner>::get();
        return runner->checkOutput("locale -a");
    }

    [[nodiscard]] bool install(std::string_view package) const override
    {
        return (cloyster::Singleton<IRunner>::get()->executeCommand(
                    fmt::format("dnf -y install {}", package))
            != 0);
    }

    [[nodiscard]] bool reinstall(std::string_view package) const override
    {
        return (cloyster::Singleton<IRunner>::get()->executeCommand(
                    fmt::format("dnf -y reinstall {}", package))
            != 0);
    }

    [[nodiscard]] bool groupInstall(std::string_view package) const override
    {
        return (cloyster::Singleton<IRunner>::get()->executeCommand(
                    fmt::format("dnf -y groupinstall \"{}\"", package))
            != 0);
    }

    [[nodiscard]] bool remove(std::string_view package) const override
    {
        return (cloyster::Singleton<IRunner>::get()->executeCommand(
                    fmt::format("dnf -y remove {}", package))
            != 0);
    }

    [[nodiscard]] bool update(std::string_view package) const override
    {
        return (cloyster::Singleton<IRunner>::get()->executeCommand(
                    fmt::format("dnf -y update {}", package))
            != 0);
    }

    [[nodiscard]] bool update() const override
    {
        return (
            cloyster::Singleton<IRunner>::get()->executeCommand("dnf -y update")
            != 0);
    }

    void check() const override
    {
        cloyster::Singleton<IRunner>::get()->executeCommand("dnf check");
    }

    void clean() const override
    {
        cloyster::Singleton<IRunner>::get()->executeCommand("dnf clean all");
    }

    [[nodiscard]] std::vector<std::string> repolist() const override
    {
        return Singleton<IRunner>::get()->checkOutput("dnf repolist");
    }

    [[nodiscard]] bool enableService(std::string_view service) const override
    {
        return Singleton<IRunner>::get()->executeCommand(
                   fmt::format("systemctl enable --now {}", service))
            == 0;
    };

    [[nodiscard]] bool disableService(std::string_view service) const override
    {
        return Singleton<IRunner>::get()->executeCommand(
                   fmt::format("systemctl disable --now {}", service))
            == 0;
    };

    [[nodiscard]] bool startService(std::string_view service) const override
    {
        return Singleton<IRunner>::get()->executeCommand(
                   fmt::format("systemctl start {}", service))
            == 0;
    };

    [[nodiscard]] bool stopService(std::string_view service) const override
    {
        return Singleton<IRunner>::get()->executeCommand(
                   fmt::format("systemctl stop {}", service))
            == 0;
    };

    [[nodiscard]] bool restartService(std::string_view service) const override
    {
        return Singleton<IRunner>::get()->executeCommand(
                   fmt::format("systemctl restart {}", service))
            == 0;
    };
};

std::unique_ptr<IOSService> IOSService::factory(const OS& osinfo)
{
    switch (osinfo.getDistro()) {
        case OS::Distro::RHEL:
        case OS::Distro::Rocky:
        case OS::Distro::AlmaLinux:
        case OS::Distro::OL:
            return makeUniqueDerived<IOSService, ELOSService>();
        default:
            throw std::logic_error("Not implemented");
    }
}
std::optional<bool> RockyLinux::m_shouldUseVault = std::nullopt;

bool RockyLinux::shouldUseVault(const OS& osinfo)
{
    const auto lastVersion = osinfo.getVersion();
    if (m_shouldUseVault) {
        return m_shouldUseVault.value();
    }
    LOG_INFO("Checking if Rocky {} should use vault", lastVersion);

    auto output = cloyster::functions::getHttpStatus(
        fmt::format("https://dl.rockylinux.org/vault/rocky/{}/BaseOS/x86_64/os/repodata/repomd.xml", lastVersion));
    const auto should = output == "200";
    LOG_INFO("{}", should ? "Yes, use vault" : "No, don't use vault");
    // Cache it
    m_shouldUseVault = std::make_optional(should);
    return should;
}

}; // namespace cloyster::services {
