#include <cloysterhpc/functions.h>
#include <cloysterhpc/utils/string.h>
#include <cloysterhpc/services/osservice.h>
#include <stdexcept>

namespace cloyster::services {
using cloyster::makeUniqueDerived;
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

bool RockyLinux::shouldUseVault(const std::string& version)
{
    auto output = cloyster::utils::getHttpStatus(
        fmt::format("https://dl.rockylinux.org/pub/rocky/{}/BaseOS/", version));
    return output == "404";
}


std::string EnterpriseLinux::repositoryURL(const std::string& repoName,
// NOLINTNEXTLINE
                   const std::string& path,
                   const std::string& upstreamUrl,
                   const bool forceUpstream)
{
    const auto opts = cloyster::Singleton<services::Options>::get();
    const auto useUpstreamUrl = opts->disableMirrors || forceUpstream;
    const auto mirrorUrl = opts->mirrorBaseUrl + "/" + repoName + "/" + path;
    // remove leading "/" if present
    auto url = cloyster::utils::string::rstrip(useUpstreamUrl ? upstreamUrl : mirrorUrl, "/");

    if (url.starts_with("file:///")) {
        auto repoPath = std::filesystem::path(url.substr(std::string_view("file:///").length()));
        if (!cloyster::exists(repoPath)) {
        throw std::runtime_error(
            fmt::format("Repository path {} does not exists, create the local repository and try again", repoPath.string()));
        }
    }

    const auto urlUpcase = cloyster::utils::string::upper(url);
    const auto isGPGKey = urlUpcase.contains("GPG") || urlUpcase.ends_with(".KEY");
    const auto status = cloyster::utils::getHttpStatus(url + (isGPGKey ? "" : "/repodata/repomd.xml"));
    const auto available = status  == "200";
    if (!available) {
        if (!useUpstreamUrl) {
            if (cloyster::utils::getHttpStatus(upstreamUrl + (isGPGKey ? "" : "/repodata/repomd.xml")) != "200") {
                // @FIXME: The execution path should only get here if the upstream URL
                //   changed during releases. I left here a message to the user
                //   about how to mitigate but it would be better if the
                //   upstream url prefix are not hardcoded at all.
                // @TODO Add an option to permit the user to override the upstream
                //   repository from the command line if required.
                throw std::runtime_error(fmt::format(
                    "Upstream repository {} is not availalble, this is "
                    "probably a bug. To mitigate copy the .repo file with the "
                    "correct url to /etc/yum.repos.d/ and try again.",
                    upstreamUrl, repoName));
            }
            LOG_WARN("Mirror {} is not available, falling back to upstream repository {}",
                     mirrorUrl, upstreamUrl);
            return upstreamUrl;
        }

        throw std::runtime_error(
            fmt::format("Repository URL {} is not available, HTTP status: {}", url, status));
    }

    return url;
}

}; // namespace cloyster::services {
