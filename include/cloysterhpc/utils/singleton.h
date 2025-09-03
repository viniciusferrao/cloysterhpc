#ifndef CLOYSTER_SERVICES_SINGLETON_H
#define CLOYSTER_SERVICES_SINGLETON_H

#include <cloysterhpc/patterns/singleton.h>
// WARNING: The getters depend on types defined in these
//   headers so we cannot use it inside these headers and
//   its transitive dependencies, sorry 
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>

/**
 * @brief Simple getter for some singletons.
 */
namespace cloyster::utils::singleton 
{

// Const singletons
constexpr auto os() { return cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS(); }
constexpr auto osservice() { return cloyster::Singleton<const services::IOSService>::get(); }
constexpr auto options() { return cloyster::Singleton<const services::Options>::get(); }
constexpr auto answerfile() { return cloyster::Singleton<const models::AnswerFile>::get(); }

// Mutable singletons
constexpr auto cluster() { return cloyster::Singleton<models::Cluster>::get(); }
constexpr auto runner() { return cloyster::Singleton<services::IRunner>::get(); }
constexpr auto repos() { return cloyster::Singleton<services::repos::RepoManager>::get(); }

}

#endif
