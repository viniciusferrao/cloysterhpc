#ifndef CLOYSTERHPC_SERVICES_OPTIONS_H_
#define CLOYSTERHPC_SERVICES_OPTIONS_H_

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cloyster::services {

struct Options final {
    bool helpAndExit;
    bool showVersion;
    bool runAsRoot;
    bool dryRun;
    bool enableTUI;
    bool enableCLI;
    bool runAsDaemon;
    bool airGap;
    bool unattended;
    std::size_t logLevelInput;
    std::string config;
    std::string helpText;
    std::string airGapUrl;
    std::string mirrorBaseUrl;
    std::string answerfile;
    std::string beegfsVersion;
    std::string dumpAnswerfile;
    std::set<std::string> skipSteps;
    std::set<std::string> forceSteps;
    std::set<std::string> ohpcPackages;

#ifndef NDEBUG
    std::string testCommand;
    std::vector<std::string> testCommandArgs;
#endif

    [[nodiscard]]
    bool shouldSkip(const std::string& step) const;
    [[nodiscard]]
    bool shouldForce(const std::string& step) const;

    static std::unique_ptr<Options> factory(int argc, const char** argv);

private:
    Options() = default;
};
}

#endif
