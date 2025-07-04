#ifndef CLOYSTERHPC_SERVICES_OPTIONS_H_
#define CLOYSTERHPC_SERVICES_OPTIONS_H_

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cloyster::services {

// @TODO Make this an aggregate type to avoid the warnings
// about partial initialization
struct Options final {
    bool parsingError;
    bool helpAndExit;
    bool showVersion;
    bool runAsRoot;
    bool dryRun;
    bool enableTUI;
    bool enableCLI;
    bool runAsDaemon;
    bool airGap;
    bool unattended;
    bool disableMirrors;
    std::size_t logLevelInput;
    std::string error;
    std::string config;
    std::string helpText;
    std::string airGapUrl;
    std::string mirrorBaseUrl;
    std::string answerfile;
    std::string beegfsVersion;
    std::string zabbixVersion;
    std::string xcatVersion;
    std::string dumpAnswerfile;
    std::string stopAfterStep;
    std::set<std::string> skipSteps;
    std::set<std::string> forceSteps;
    std::set<std::string> ohpcPackages;

    std::string testCommand;
    std::vector<std::string> testCommandArgs;

    [[nodiscard]]
    bool shouldSkip(const std::string& step) const;
    [[nodiscard]]
    bool shouldForce(const std::string& step) const;

    void maybeStopAfterStep(const std::string& step) const;

};
static_assert(std::is_aggregate_v<Options>, "Options must be an aggregate type.");

namespace options {
std::unique_ptr<Options> factory(int argc, const char** argv);
};
}

#endif
