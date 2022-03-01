//
// Created by Vinícius Ferrão on 24/11/21.
//

#ifndef CLOYSTERHPC_PRESENTER_H
#define CLOYSTERHPC_PRESENTER_H

#include "../view/newt.h"
#include "../cluster.h"
#include "../services/log.h"

#include "../include/magic_enum.hpp"

#include <memory>
#include <variant>
#include <fmt/format.h>
#include <string_view>

#include <boost/lexical_cast.hpp>

// TODO: Pure virtual
class Presenter {
private:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    // TODO: This should be on separate classes
    struct Messages {
        struct Placeholder {
            static constexpr const char* help = "We're sorry, but there's no help available for this screen";
        };

        struct Welcome {
            static constexpr const char* message =
                "Welcome to the guided installer of CloysterHPC!\n\nLorem ipsum dolor " \
                "sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt " \
                "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. " \
                "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum " \
                "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non " \
                "proident, sunt in culpa qui officia deserunt mollit anim id est laborum." \
                "\n";
        };

        struct GuidedInstall {
            static constexpr const char* message =
                "We will start the installation phase now.\n\nYou will be guided through " \
                "a series of questions about your HPC cluster so we can configure " \
                "everything accordingly to your needs. You're free to cancel the process " \
                "at any given time.\n";
        };

        struct HostId {
            static constexpr const char* question = "Enter the desired hostname and domain name for this machine";
            static constexpr const char* help = Placeholder::help;
        };
    };

    void welcomeMessage();
    void installInstructions();
    std::string timezoneSelection(const std::vector<std::string>&);

    // TODO: Try to return an string_view instead
    template<size_t N>
    std::string localeSelection(const std::array<std::string_view, N>& locales);

//    std::vector<std::string> networkHostnameSelection(const std::vector<std::string>&);

    // TODO: Organize this code
    template<size_t N>
    std::array<std::pair<std::string, std::string>, N>
    networkHostnameSelection(const std::array<std::pair<std::string, std::string>, N>& entries);

public:
    Presenter(std::unique_ptr<Newt>& view,
              std::unique_ptr<Cluster>& model);


};

#endif //CLOYSTERHPC_PRESENTER_H
