#include <cloysterhpc/services/options.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

namespace cloyster::services {

std::unique_ptr<Options> Options::factory(int argc, const char** argv)
{
    namespace po = boost::program_options;

    // Create an instance of Options with default values
    Options opt;
    opt.helpAndExit = false;
    opt.showVersion = false;
    opt.runAsRoot = false;
    opt.dryRun = false;
    opt.enableTUI = false;
    opt.enableCLI = false;
    opt.runAsDaemon = false;
    opt.airGap = false;         // Explicitly set to false
    opt.airGapUrl = "";         // Default to empty string
    opt.mirrorBaseUrl = "";     // Default to empty string
    opt.logLevelInput = 3;    // Default log level
    opt.answerfile = "";        // Default to empty string
    opt.unattended = false;     // Added from main.cpp
    opt.dumpAnswerfile = "";    // Added from main.cpp
#ifndef NDEBUG
    opt.testCommand = "";       // Added from main.cpp (debug only)
    opt.testCommandArgs = {};   // Added from main.cpp (debug only)
#endif

    // Define the options description
    po::options_description desc("CloysterHPC Options");
    desc.add_options()
        ("help,h", po::bool_switch(&opt.helpAndExit), "Show help message")
        ("version,v", po::bool_switch(&opt.showVersion), "Show version information")
        ("root,r", po::bool_switch(&opt.runAsRoot), "Run as root")
        ("dry,d", po::bool_switch(&opt.dryRun), "Perform a dry run installation")
        ("tui,t", po::bool_switch(&opt.enableTUI), "Enable TUI")
        ("cli,c", po::bool_switch(&opt.enableCLI), "Enable CLI")
        ("daemon,D", po::bool_switch(&opt.runAsDaemon), "Run as daemon")
        ("airgap,g", po::bool_switch(&opt.airGap), "Enable air-gapped installation")
        ("airgap-url", po::value<std::string>(&opt.airGapUrl)
             ->default_value("file:///var/repos/"), "URL for air-gapped installation")
        ("mirror-url", po::value<std::string>(&opt.mirrorBaseUrl)
             ->default_value("https://mirror.versatushpc.com.br"), "Base URL for mirror")
        ("beegefs-version", po::value<std::string>(&opt.beegfsVersion)
             ->default_value("beegfs_7.3.3"), "BeeGFS default version")
        ("log-level,l", po::value<std::size_t>(&opt.logLevelInput)->default_value(3)
         ->notifier([](int val) {
         if (val < 1 || val > 6) {
         throw po::validation_error(po::validation_error::invalid_option_value,
                                    "log-level", std::to_string(val));
         }
         }), 
         "Set log level (integer between 1 and 6)")
        ("answerfile,a", po::value<std::string>(&opt.answerfile), "Full path to an answerfile")
        ("skip", po::value<std::vector<std::string>>()->multitoken()->composing(), "Skip specific steps during installation")
        ("force", po::value<std::vector<std::string>>()->multitoken()->composing(), "Force specific steps during installation")
        ("ohpc-packages", po::value<std::vector<std::string>>()->multitoken()->composing(), "Select OHPC packages")
        ("unattended,u", po::bool_switch(&opt.unattended), "Perform an unattended installation") // Added from main.cpp
        ("dump-answerfile", po::value<std::string>(&opt.dumpAnswerfile), 
            "Create an answerfile based on input and save to specified path") // Added from main.cpp
#ifndef NDEBUG
        ("test", po::value<std::string>(&opt.testCommand), "Run a command for testing purposes") // Added from main.cpp
        ("test-args", po::value<std::vector<std::string>>(&opt.testCommandArgs)->multitoken()->composing(), 
            "Arguments for test command") // Added from main.cpp
#endif
    ;

    // Parse the command-line arguments (Boost will handle errors by exiting)
    po::variables_map vmap;
    po::store(po::parse_command_line(argc, argv, desc), vmap);
    po::notify(vmap);

    // Handle help (print message but continue processing)
    if (vmap.contains("help")) {
        std::ostringstream stream;
        stream << desc;
        opt.helpText = stream.str();
    }

    constexpr auto initializeSetOption = [](const auto key, auto& output, auto& vmap,
                                            const std::initializer_list<const char *>& defaultValue = {}){
        if (vmap.contains(key)) {
            const auto& vec = vmap[key].template as<std::vector<std::string>>();

            output.insert(vec.begin(), vec.end());
        } else {
            output.insert(defaultValue.begin(), defaultValue.end());
        }
    };

    initializeSetOption("skip", opt.skipSteps, vmap);
    initializeSetOption("force", opt.forceSteps, vmap);
    const auto ohpcPackagesDefault = {
        "openmpi4-gnu12-ohpc",
        "mpich-ofi-gnu12-ohpc",
        "mpich-ucx-gnu12-ohpc",
        "mvapich2-gnu12-ohpc",
        "lmod-defaults-gnu12-openmpi4-ohpc",
        "ohpc-autotools",
        "hwloc-ohpc",
        "spack-ohpc",
        "valgrind-ohpc"};
    initializeSetOption("ohpc-packages", opt.ohpcPackages, vmap, ohpcPackagesDefault);

    // Validate log-level input using std::from_chars (set to default "3" if invalid)
    int levelNum = 0;
    constexpr int levelMax = 6;
    constexpr int levelMin = 1;

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

};
