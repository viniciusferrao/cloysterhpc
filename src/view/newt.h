//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWT_H
#define CLOYSTER_NEWT_H

#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include <iostream>
#include <newt.h>
#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include "view.h"
#include "../services/log.h"
#include "../functions.h"

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class Newt : public View {
private:
    struct TUIText {

#if __cpp_lib_constexpr_string >= 201907L
        static constexpr const char* title =
                fmt::format("{} Installer", productName).c_str();
#else
        static constexpr const char* title = PRODUCT_NAME " Installer";
#endif
        static constexpr const char* helpLine = "  <Tab>/<Alt-Tab> between elements   |  <Space> selects   |  <F12> disabled";
        static constexpr const char* abort = "Installation aborted due to operator request";

        struct Buttons {
            static constexpr const char* ok = "OK";
            static constexpr const char* cancel = "Cancel";
            static constexpr const char* yes = "Yes";
            static constexpr const char* no = "No";
            static constexpr const char* help = "Help";
        };

        struct Help {
            static constexpr const char* title = "Help";
        };
    };

protected:
    int m_cols;
    int m_rows;
    int m_suggestedWidth;
    int m_flexDown;
    int m_flexUp;
    int m_dataWidth;
    int m_maxListHeight;

    void abort () override;
    void helpMessage (const char*) override;
    bool hasEmptyField (const struct newtWinEntry*);

public:
    Newt();

    Newt(const Newt&) = delete;
    Newt& operator=(const Newt&) = delete;

    Newt(Newt&&) = delete;
    Newt& operator=(Newt&&) = delete;
    
    ~Newt() override;

    void message(const char*);
    void message(const char*, const char*);

    void okCancelMessage(const char* message);
    void okCancelMessage(const char* title, const char* message);

    // TODO:
    //  * Better template?
    //  * The name "okCancelMessage" of this function is not ideal
    template<std::size_t N>
    void okCancelMessage(const char* title, const char* message,
                         const std::array<std::pair<std::string, std::string>, N>& pairs)
    {
        int returnValue;
        std::string newMessage = message;

        newMessage += "\n\n";
        for (auto const& pair : pairs) {
            newMessage += fmt::format("{} -> {}\n", pair.first, pair.second);
        }

        returnValue = newtWinChoice(const_cast<char*>(title),
                                    const_cast<char*>(TUIText::Buttons::ok),
                                    const_cast<char*>(TUIText::Buttons::cancel),
                                    const_cast<char*>(newMessage.c_str()));

        switch (returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                break;
            case 2:
                abort();
                break;
            default:
                throw std::runtime_error("Out of bounds in a switch statement");
        }
    }

    template<std::size_t N>
    void okCancelMessage(const char* message,
                         const std::array<std::pair<std::string, std::string>, N>& pairs) {
        okCancelMessage(nullptr, message, pairs);
    }

    // TODO:
    //  * Add C++20 concepts; limit by some types.
    //  * Optimize for std::string_view and std::string.
    template<typename T>
    std::string listMenu (const char* title, const char* message,
                          const T& items,
                          const char* helpMessage)
    {
        int returnValue;
        // TODO: Initial value of selector should be available on function
        //       declaration so we can start at the already set option
        int selector = 0;

        // TODO: Is it possible do use std::array instead?
        // TODO: Check types to avoid this copy (C++20 concepts?)
        std::vector<std::string> tempStrings(items.begin(), items.end());

        // Newt expects a NULL terminated array of C style strings
        std::vector<const char*> cStrings;
        cStrings.reserve(tempStrings.size() + 1);

        for (const auto& string : tempStrings) {
            cStrings.push_back(string.c_str());
            LOG_TRACE("Pushed back std::string {}", string.c_str());
        }
        cStrings.push_back(nullptr);
        LOG_TRACE("Pushed back nullptr");

#if 1
        // goto implementation
        question:
        returnValue = newtWinMenu(const_cast<char*>(title),
                                  const_cast<char*>(message),
                                  m_suggestedWidth,
                                  m_flexDown,
                                  m_flexUp,
                                  m_maxListHeight,
                                  const_cast<char**>(cStrings.data()),
                                  &selector,
                                  const_cast<char*>(TUIText::Buttons::ok),
                                  const_cast<char*>(TUIText::Buttons::cancel),
                                  const_cast<char*>(TUIText::Buttons::help),
                                  nullptr);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return tempStrings[boost::lexical_cast<std::size_t>(selector)];
            case 2:
                abort();
                break;
            case 3:
                this->helpMessage(helpMessage);
                goto question;
            default:
                __builtin_unreachable();
        }
#else
        // gotoless implementation
    for (;;) {
        returnValue = newtWinMenu(const_cast<char*>(title),
                                  const_cast<char*>(message),
                                  m_suggestedWidth,
                                  m_flexUp,
                                  m_flexDown,
                                  m_maxListHeight,
                                  const_cast<char**>(cStrings.data()),
                                  &selector,
                                  const_cast<char*>(TUIText::Buttons::ok),
                                  const_cast<char*>(TUIText::Buttons::cancel),
                                  const_cast<char*>(TUIText::Buttons::help),
                                  NULL);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return items[selector];
            case 2:
                abortInstall();
            case 3:
                this->helpMessage(helpMessage);
                continue;
        }
        break; // for (;;)
    }
#endif
        __builtin_unreachable();
    }

    /**
     * Show a progress message dialog
     * @param title
     * @param message
     * @param command
     * @param fPercent A function to transform a line
     * into a percent (a 0 to 100 value)
     */
    bool progressMenu(const char* title,
                      const char* message,
                      cloyster::CommandProxy&&,
                      std::function<double(std::string)> fPercent);

    // TODO:
    //  * Add C++20 concepts; limit by some types.
    //  * Optimize for std::string_view and std::string.
    //  * std::optional on second pair
    template<typename T>
    T fieldMenu(const char* title, const char* message,
                const T& items,
                const char* helpMessage)
    {
        int returnValue;

        std::size_t arraySize = items.size();
        auto fieldEntries = std::make_unique<char*[]>(arraySize + 1);
        auto field = std::make_unique<newtWinEntry[]>(arraySize + 1);

        // This "for loop" will populate newtWinEntry with the necessary data to be
        // displayed on the interface.
        // Please note that field[i].value is a char** because it's passing data by
        // reference in C style, since the data can be modified by the newt form,
        // it's not an array of char*
        for (std::size_t i = 0 ; i < arraySize ; i++) {
            field[i].text = const_cast<char*>(items[i].first.c_str());
            fieldEntries[i] = const_cast<char*>((items[i].second).c_str());
            LOG_TRACE("fieldEntries[{}] = {}", i, fieldEntries[i]);

            // TODO: Check is there's a way to hide &
            field[i].value = &fieldEntries[i];

            // FIXME: Fix this hack to enable password fields
            if (items[i].first.find("Password") != std::string::npos ||
                items[i].first.find("password") != std::string::npos)
                field[i].flags = NEWT_FLAG_PASSWORD;
            else
                field[i].flags = 0;
        }

        field[arraySize].text = nullptr;
        field[arraySize].value = nullptr;
        field[arraySize].flags = 0;

        T returnArray;

        question:
        returnValue = newtWinEntries(const_cast<char*>(title),
                                     const_cast<char*>(message),
                                     m_suggestedWidth,
                                     m_flexDown,
                                     m_flexUp,
                                     m_dataWidth,
                                     field.get(),
                                     const_cast<char*>(TUIText::Buttons::ok),
                                     const_cast<char*>(TUIText::Buttons::cancel),
                                     const_cast<char*>(TUIText::Buttons::help),
                                     nullptr);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                // TODO: The view should now check for this, it's a passive view
                if (hasEmptyField(field.get()))
                    goto question;

                // FIXME: We forgot that we should return size_t sometimes and
                //        that was triggering an exception on the presenter, so
                //        basically the std::variant is useless here, we always
                //        return std:string.
                for (std::size_t i = 0 ; field[i].text ; i++) {
                    returnArray[i] =
                            std::make_pair<std::string, std::string>(
                                    field[i].text, *field[i].value);
                }

                return returnArray;
            case 2:
                abort();
                break;
            case 3:
                this->helpMessage(helpMessage);
                goto question;
            default:
                throw std::runtime_error(
                        "Invalid return value from fields on newt library");
        }
        throw std::runtime_error("Invalid return path on newt library");
    }

    bool yesNoQuestion(const char* title, const char* message,
                       const char* helpMessage);

};

#endif //CLOYSTER_NEWT_H
