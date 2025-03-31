#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/functions.h>
#include <stdexcept>

namespace cloyster::services {
using cloyster::makeUniqueDerived;
using cloyster::Singleton;

class ELOSService final : public IOSService {
public:
    [[nodiscard]] std::string getKernelInstalled() const override
    {
        auto runner = Singleton<IRunner>::get();
        // Get the kernel version from the kernel package, order by BUILDTIME since
        // there may be multiple kernels installed (previous kernels)
        return runner->checkOutput("bash -c \"rpm -q kernel --qf '%{VERSION}-%{RELEASE} %{BUILDTIME}\n' | sort -nrk 2 | head -1 | awk '{print $1}'\"")[0];
    }

    [[nodiscard]] std::string getKernelRunning() const override
    {
        return Singleton<IRunner>::get()->checkOutput("uname -r")[0];
    }
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

}; // namespace cloyster::services {
