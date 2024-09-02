#include <cloysterhpc/cloyster.h>

namespace cloyster {
bool showVersion = false;
bool runAsRoot = false;
bool dryRun = false;
bool enableTUI = false;
bool enableCLI = false;
bool runAsDaemon = false;

std::string logLevelInput
    = fmt::format("{}", magic_enum::enum_name(Log::Level::Info));
std::string answerfile = "";
std::string customRepofilePath = "";
}
