#include <cloysterhpc/services/options.h>

#include <CLI/CLI.hpp>
#include <fstream>
#include <memory>
#include <set>
#include <vector>

namespace cloyster::services {
using std::ifstream;

std::unique_ptr<Options> options::factory(int argc, const char** argv)
{
    // Create an instance of Options with default values
    Options opt;
    opt.helpAndExit = false;
    opt.showVersion = false;
    opt.runAsRoot = false;
    opt.dryRun = false;
    opt.enableTUI = false;
    opt.enableCLI = false;
    opt.runAsDaemon = false;
    opt.airGap = false;
    opt.airGapUrl = "file:///var/repos/";
    opt.mirrorBaseUrl = "https://mirror.versatushpc.com.br";
    opt.beegfsVersion = "beegfs_7.3.3";
    opt.zabbixVersion = "6.4";
    opt.xcatVersion = "latest";
    opt.logLevelInput = 3;
    opt.answerfile = "";
    opt.unattended = false;
    opt.dumpAnswerfile = "";
    opt.config = "";
#ifndef NDEBUG
    opt.testCommand = "";
    opt.testCommandArgs = {};
#endif

    // Define the CLI11 app
    CLI::App app("CloysterHPC Options");

    // Add options
    app.add_flag("-v,--version", opt.showVersion, "Show version information");
    app.add_flag("-r,--root", opt.runAsRoot, "Run as root");
    app.add_flag("-d,--dry", opt.dryRun, "Perform a dry run installation");
    app.add_flag("-t,--tui", opt.enableTUI, "Enable TUI");
    app.add_flag("-c,--cli", opt.enableCLI, "Enable CLI");
    app.add_flag("-D,--daemon", opt.runAsDaemon, "Run as daemon");
    app.add_flag("--disable-mirrors", opt.disableMirrors, "Disable mirror URLs");
    app.add_option("--mirror-url", opt.mirrorBaseUrl, "Base URL for mirror")
        ->default_str("https://mirror.versatushpc.com.br");
    app.add_option("--beegfs-version", opt.beegfsVersion, "BeeGFS default version")
        ->default_str("beegfs_7.3.3");
    app.add_option("--xcat-version", opt.beegfsVersion, "xCAT default version")
        ->default_str("latest");
    app.add_option("--zabbix-version", opt.zabbixVersion, "Zabbix default version")
        ->default_str("6.4");
    app.add_option("-l,--log-level", opt.logLevelInput, "Set log level (integer between 1 and 6)")
        ->default_val(3)
        ->check(CLI::Range(1, 6));
    app.add_option("-a,--answerfile", opt.answerfile, "Full path to an answerfile");
    app.add_option("--skip", opt.skipSteps, "Skip specific steps during installation")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    app.add_option("--force", opt.forceSteps, "Force specific steps during installation")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    app.add_option("--ohpc-packages", opt.ohpcPackages, "Select OHPC packages")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    app.add_option("--repos", opt.enabledRepos, "Enabled repositories")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    app.add_flag("-u,--unattended", opt.unattended, "Perform an unattended installation");
    app.add_option("--dump-answerfile", opt.dumpAnswerfile, "Create an answerfile based on input and save to specified path");
    app.add_option("--config", opt.config, "Config file to pass options for the command line from a configuration file");

#ifndef NDEBUG
    app.add_option("--test", opt.testCommand, "Run a command for testing purposes");
    app.add_option("--test-args", opt.testCommandArgs, "Arguments for test command")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
#endif

    // Parse command-line arguments
    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp& e) {
        opt.helpText = app.help();
        opt.helpAndExit = true;
    } catch (const CLI::CallForVersion& e) {
        opt.showVersion = true;
    } catch (const CLI::ParseError& e) {
        opt.parsingError = true;
        opt.error = e.what();
    }

    // Handle configuration file if specified
    if (!opt.config.empty()) {
        std::ifstream configFile(opt.config);
        if (!configFile.is_open()) {
            throw CLI::FileError::Missing(opt.config);
        }
        std::string configContent((std::istreambuf_iterator<char>(configFile)),
                                  std::istreambuf_iterator<char>());
        std::istringstream configStream(configContent);
        try {
            app.parse_from_stream(configStream);
        } catch (const CLI::ParseError& e) {
            opt.parsingError = true;
            opt.error = e.what();
        }
    }


    // Initialize sets with default values if not provided
    if (opt.ohpcPackages.empty()) {
        opt.ohpcPackages = { "openmpi4-gnu12-ohpc", "mpich-ofi-gnu12-ohpc",
                             "mpich-ucx-gnu12-ohpc", "mvapich2-gnu12-ohpc",
                             "lmod-defaults-gnu12-openmpi4-ohpc", "ohpc-autotools",
                             "hwloc-ohpc", "spack-ohpc", "valgrind-ohpc" };
    }
    if (opt.enabledRepos.empty()) {
        opt.enabledRepos = { "beegfs", "elrepo", "epel", "OpenHPC",
                             "OpenHPC-Updates", "rpmfusion" };
    }

    return std::make_unique<Options>(std::move(opt));
}

bool Options::shouldSkip(const std::string& step) const
{
    return skipSteps.contains(step);
}

bool Options::shouldForce(const std::string& step) const
{
    return forceSteps.contains(step);
}

} // namespace cloyster::services
