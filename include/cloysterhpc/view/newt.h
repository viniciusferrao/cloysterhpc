/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NEWT_H_
#define CLOYSTERHPC_NEWT_H_

#include <boost/lexical_cast.hpp>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/view/view.h>
#include <fmt/format.h>
#include <iostream>
#include <newt.h>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class Newt : public View {
private:
    struct TUIText {
        // TODO: Get rid of the macro: https://stackoverflow.com/q/75216821
        static constexpr const auto title = PRODUCT_NAME " Installer";
        static constexpr const auto version = "Version " PRODUCT_VERSION;
#ifndef NDEBUG
        static constexpr const auto developmentBuild = "DEVELOPMENT BUILD";
#endif
        static constexpr const auto helpLine
            = "  <Tab>/<Alt-Tab> between elements   |  <Space> selects   |  "
              "<F12> disabled";
        static constexpr const auto abort
            = "Installation aborted due to operator request";

        struct Buttons {
            static constexpr const auto ok = "OK";
            static constexpr const auto cancel = "Cancel";
            static constexpr const auto yes = "Yes";
            static constexpr const auto add = "Add";
            static constexpr const auto remove = "Remove";
            static constexpr const auto no = "No";
            static constexpr const auto help = "Help";
        };

        struct Help {
            static constexpr const auto title = "Help";
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

    void abort() override;
    void helpMessage(const char*) override;
    bool hasEmptyField(const struct newtWinEntry*);

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

    std::pair<int, std::vector<std::string>> multipleSelectionMenu(
        const char* title, const char* message, const char* help,
        std::vector<std::tuple<std::string, std::string, bool>> items);

    // TODO:
    //  * Better template?
    //  * The name "okCancelMessage" of this function is not ideal
    template <std::size_t N>
    void okCancelMessage(const char* title, const char* message,
        const std::array<std::pair<std::string, std::string>, N>& pairs)
    {
        int returnValue;
        std::string newMessage = message;

        newMessage += "\n\n";
        for (const auto& pair : pairs) {
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

    template <std::size_t N>
    void okCancelMessage(const char* message,
        const std::array<std::pair<std::string, std::string>, N>& pairs)
    {
        okCancelMessage(nullptr, message, pairs);
    }

    std::vector<const char*> convertToNewtList(
        const std::vector<std::string>& s);

    // TODO:
    //  * Optimize for std::string_view and std::string.

    using ListButtonCallback = std::function<bool(std::vector<std::string>&)>;

    /**
     * Add a form screen containing a list of things, used when you need every
     * item of that list, and not only one.
     *
     * @param title The window title
     *
     * @param message The message below the title
     *
     * @param items The items to be shown
     *
     * @param addCallback A callback function that is executed when we click on
     * the Add button. It receives a mutable reference to the items as a
     * parameter, and returns a boolean, the value of whether we should continue
     * running the form (true) or not (false)
     */
    template <std::ranges::range T>
    std::vector<std::string> collectListMenu(const char* title,
        const char* message, const T& items, const char* helpMessage,
        ListButtonCallback&& addCallback)
    {
        int returnValue;
        // TODO: Initial value of selector should be available on function
        //       declaration so we can start at the already set option
        int selector = 0;

        // TODO: Check types to avoid this copy (C++20 concepts?)
        std::vector<std::string> tempStrings(
            std::begin(items), std::end(items));

        auto cStrings = convertToNewtList(tempStrings);

    // goto implementation
    question:
        returnValue = newtWinMenu(const_cast<char*>(title),
            const_cast<char*>(message), m_suggestedWidth, m_flexDown, m_flexUp,
            m_maxListHeight, const_cast<char**>(cStrings.data()), &selector,
            const_cast<char*>(TUIText::Buttons::ok),
            const_cast<char*>(TUIText::Buttons::cancel),
            const_cast<char*>(TUIText::Buttons::add),
            const_cast<char*>(TUIText::Buttons::remove),
            const_cast<char*>(TUIText::Buttons::help), nullptr);

        switch (returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return tempStrings;
            case 2:
                abort();
                break;
            case 3: { // add
                bool ret = addCallback(tempStrings);
                if (ret) {
                    cStrings = convertToNewtList(tempStrings);
                    goto question;
                } else {
                    return tempStrings;
                }
            }
            case 4: // remove
                if (selector >= 0 && selector < cStrings.size()) {
                    tempStrings.erase(tempStrings.begin() + selector);
                    cStrings = convertToNewtList(tempStrings);
                }

                goto question;
            case 5:
                this->helpMessage(helpMessage);
                goto question;
            default:
                __builtin_unreachable();
        }
    }

    template <std::ranges::range T>
    std::string listMenu(const char* title, const char* message, const T& items,
        const char* helpMessage)
    {
        int returnValue;
        // TODO: Initial value of selector should be available on function
        //       declaration so we can start at the already set option
        int selector = 0;

        // TODO: Is it possible do use std::array instead?
        // TODO: Check types to avoid this copy (C++20 concepts?)
        std::vector<std::string> tempStrings(
            std::begin(items), std::end(items));

        // Newt expects a NULL terminated array of C style strings
        std::vector<const char*> cStrings = convertToNewtList(tempStrings);

#if 1
    // goto implementation
    question:
        returnValue = newtWinMenu(const_cast<char*>(title),
            const_cast<char*>(message), m_suggestedWidth, m_flexDown, m_flexUp,
            m_maxListHeight, const_cast<char**>(cStrings.data()), &selector,
            const_cast<char*>(TUIText::Buttons::ok),
            const_cast<char*>(TUIText::Buttons::cancel),
            const_cast<char*>(TUIText::Buttons::help), nullptr);

        switch (returnValue) {
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
                const_cast<char*>(message), m_suggestedWidth, m_flexUp,
                m_flexDown, m_maxListHeight,
                const_cast<char**>(cStrings.data()), &selector,
                const_cast<char*>(TUIText::Buttons::ok),
                const_cast<char*>(TUIText::Buttons::cancel),
                const_cast<char*>(TUIText::Buttons::help), NULL);

            switch (returnValue) {
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
    bool progressMenu(const char* title, const char* message,
        cloyster::CommandProxy&&, std::function<double(std::string)> fPercent);

    // TODO:
    //  * Optimize for std::string_view and std::string.
    //  * std::optional on second pair
    template <std::ranges::range T>
    T fieldMenu(const char* title, const char* message, const T& items,
        const char* helpMessage)
    {
        int returnValue;

        std::size_t arraySize = items.size();
        auto fieldEntries = std::make_unique<char*[]>(arraySize + 1);
        auto field = std::make_unique<newtWinEntry[]>(arraySize + 1);

        // Yes, it's like this. std::views::enumerate isn't supported on clang
        // or gcc < 13.
        for (size_t i = 0; const auto& item : items) {
            field[i].text = const_cast<char*>(item.first.c_str());
            fieldEntries[i] = const_cast<char*>((item.second).c_str());
            LOG_TRACE("fieldEntries[{}] = {}", i, fieldEntries[i])

            // TODO: Check is there's a way to hide &
            field[i].value = &fieldEntries[i];

            // FIXME: Fix this hack to enable password fields
            if (item.first.contains("Password")
                || item.first.contains("password"))
                field[i].flags = NEWT_FLAG_PASSWORD;
            else
                field[i].flags = 0;

            i++;
        }

        field[arraySize].text = nullptr;
        field[arraySize].value = nullptr;
        field[arraySize].flags = 0;

        T returnArray;

    question:
        returnValue = newtWinEntries(const_cast<char*>(title),
            const_cast<char*>(message), m_suggestedWidth, m_flexDown, m_flexUp,
            m_dataWidth, field.get(), const_cast<char*>(TUIText::Buttons::ok),
            const_cast<char*>(TUIText::Buttons::cancel),
            const_cast<char*>(TUIText::Buttons::help), nullptr);

        switch (returnValue) {
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
                for (std::size_t i = 0; field[i].text; i++) {
                    returnArray[i] = std::make_pair<std::string, std::string>(
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

    bool yesNoQuestion(
        const char* title, const char* message, const char* helpMessage);
};

#endif // CLOYSTERHPC_NEWT_H_
