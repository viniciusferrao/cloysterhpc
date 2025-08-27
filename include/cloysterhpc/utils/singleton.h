#ifndef CLOYSTER_SERVICES_SINGLETON_H
#define CLOYSTER_SERVICES_SINGLETON_H

#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>

namespace cloyster::utils::singleton 
{

// Simpler singleton getters to keep code clean
constexpr auto cluster() { return cloyster::Singleton<models::Cluster>::get(); }
constexpr auto os() { return cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS(); }
constexpr auto runner() { return cloyster::Singleton<services::IRunner>::get(); }
constexpr auto osservice() { return cloyster::Singleton<services::IOSService>::get(); }
constexpr auto repos() { return cloyster::Singleton<services::repos::RepoManager>::get(); }
constexpr auto options() { return cloyster::Singleton<services::Options>::get(); }

}

#endif
