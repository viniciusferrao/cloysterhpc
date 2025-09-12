#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/init.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/utils/singleton.h>

#include <cloysterhpc/dbus_client.h>
#include <cloysterhpc/messagebus.h>

namespace cloyster::services {

using namespace cloyster;
using namespace cloyster::services;

// WARNING: If you change the type T in Singleton<T>::init(...) (to const T for
// instance) all the Singleton<T>::get need to be changed, otherwise you get
// "Singleton read before initialization error" at runtime. While there are
// getters to handle this in cloyster/utils/singletons in a uniform way and for
// most cases, these getters depends on headers (that introduce the type
// T in question), so files including the same header cannot use these getters
// (or we have recursive header inclusion error).

// Singletons that depends only in the options, the cluster model
// depends on these
void initializeSingletonsOptions(std::unique_ptr<const Options>&& opts)
{
    Singleton<const Options>::init(std::move(opts));
    cloyster::Singleton<MessageBus>::init([]() {
        return cloyster::functions::makeUniqueDerived<MessageBus, DBusClient>(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1");
    });
    cloyster::Singleton<cloyster::services::IRunner>::init([&]() {
        using cloyster::services::IRunner;
        using cloyster::services::DryRunner;
        using cloyster::services::Runner;
        auto opts = Singleton<const Options>::get();

        if (opts->dryRun) {
            return cloyster::functions::makeUniqueDerived<IRunner, DryRunner>();
        }

        return cloyster::functions::makeUniqueDerived<IRunner, Runner>();
    });
}

// Singletons that depends on the cluster model
void initializeSingletonsModel(
    std::unique_ptr<cloyster::models::Cluster>&& cluster,
    std::unique_ptr<const cloyster::models::AnswerFile>&& answerfile)
{
    using cloyster::models::Cluster;
    cloyster::Singleton<const models::AnswerFile>::init(std::move(answerfile));
    cloyster::Singleton<Cluster>::init(std::move(cluster));

    using cloyster::services::repos::RepoManager;
    cloyster::Singleton<RepoManager>::init([]() {
        auto repoManager = std::make_unique<RepoManager>();
        return repoManager;
    });

    cloyster::Singleton<const cloyster::services::IOSService>::init([]() {
        const auto& osinfo
            = cloyster::Singleton<Cluster>::get()->getHeadnode().getOS();
        return cloyster::services::IOSService::factory(osinfo);
    });
}

}; // namespace cloyster::services
