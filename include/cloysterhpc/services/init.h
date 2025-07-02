#include <cloysterhpc/services/options.h>
#include <cloysterhpc/models/cluster.h>

namespace cloyster::services {
using namespace cloyster;

// Singletons that depends only in the options, the cluster model
// depends on these
void initializeSingletonsOptions(std::unique_ptr<Options>&& opts);


// Singletons that depends on the cluster model
void initializeSingletonsModel(std::unique_ptr<cloyster::models::Cluster>&& cluster);
}
