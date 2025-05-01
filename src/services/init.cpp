#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/init.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/functions.h>

#include <cloysterhpc/messagebus.h>
#include <cloysterhpc/dbus_client.h>

namespace cloyster::services {

using namespace cloyster;
using namespace cloyster::services;

// Singletons that depends only in the options, the cluster model
// depends on these
void initializeSingletonsOptions(std::unique_ptr<Options>&& opts)
{
    Singleton<Options>::init(std::move(opts));
    cloyster::Singleton<MessageBus>::init([]() {
        return cloyster::makeUniqueDerived<MessageBus, DBusClient>(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1");
    });
    cloyster::Singleton<cloyster::services::IRunner>::init([&]() {
        using cloyster::services::IRunner;
        using cloyster::services::DryRunner;
        using cloyster::services::Runner;
        auto opts = Singleton<Options>::get();

        if (opts->dryRun) {
            return cloyster::makeUniqueDerived<IRunner, DryRunner>();
        }

        return cloyster::makeUniqueDerived<IRunner, Runner>();
    });
}

// Singletons that depends on the cluster model
void initializeSingletonsModel(std::unique_ptr<cloyster::models::Cluster>&& cluster)
{
    using cloyster::models::Cluster;
    cloyster::Singleton<Cluster>::init(std::move(cluster));

    using cloyster::services::repos::RepoManager;
    cloyster::Singleton<RepoManager>::init([]() {
        auto clusterPtr = cloyster::Singleton<Cluster>::get();
        const auto& osinfo = clusterPtr->getHeadnode().getOS();
        auto repoManager = std::make_unique<RepoManager>();
        return repoManager;
    });

    cloyster::Singleton<cloyster::services::IOSService>::init([]() {
        const auto& osinfo
            = cloyster::Singleton<Cluster>::get()->getHeadnode().getOS();
        return cloyster::services::IOSService::factory(osinfo);
    });
}

}; // namespace cloyster::services
