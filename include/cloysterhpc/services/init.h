#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/options.h>

namespace cloyster::services {
using namespace cloyster;

// Singletons that depends only in the options, the cluster model
// depends on these
void initializeSingletonsOptions(std::unique_ptr<const Options>&& opts);

void initializeSingletonsModel(
    std::unique_ptr<cloyster::models::Cluster>&& cluster,
    std::unique_ptr<const cloyster::models::AnswerFile>&& answerfile);

}
