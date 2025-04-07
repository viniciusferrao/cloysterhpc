#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/functions.h>

namespace cloyster {
// Globals, intialized by the command line parser
// @FIXME: Wrap up this in an Options type and replace CLI11 with boost
bool showVersion = false;
bool runAsRoot = false;
bool dryRun = false;
bool enableTUI = false;
bool enableCLI = false;
bool runAsDaemon = false;
bool airGap = false;
std::string airGapUrl = "file:///var/repos/";
std::string mirrorBaseUrl = "https://mirror.versatushpc.com.br"; // Global mirror URL
std::string beegfsVersion = "beegfs_7.3.3"; // Default BeeGFS version, configurable via command-line

std::string logLevelInput
    = fmt::format("{}", cloyster::utils::enums::toString(Log::Level::Info));
std::string answerfile = "";
std::string customRepofilePath = "";

std::set<std::string> skipSteps {};
std::set<std::string> forceSteps {};

bool shouldSkip(const std::string& step) { return skipSteps.contains(step); }

bool shouldForce(const std::string& step) { return forceSteps.contains(step); }

}
