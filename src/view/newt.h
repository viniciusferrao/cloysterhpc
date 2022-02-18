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
#include "view.h"
#include "../messages.h" /* Legacy constexpr */
#include "../services/log.h"

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class Newt : public View {
protected:
    void abortInstall () override;
    void helpMessage (const char*) override;
    bool hasEmptyField (const struct newtWinEntry*);

public:
    Newt();

    Newt(Newt const& other) = delete;
    Newt& operator=(Newt const& other) = delete;

    Newt(Newt&& other) = delete;
    Newt& operator=(Newt&& other) = delete;
    
    ~Newt() override;

    void message(const char*);
    void message(const char*, const char*);

    void okCancelMessage(const char* message);
    void okCancelMessage(const char* title, const char* message);
    // TODO: Template?
    void okCancelMessage(const char* message, const std::vector<std::pair<std::string, std::string>>& pairs);
    void okCancelMessage(const char* title, const char* message, const std::vector<std::pair<std::string, std::string>>& pairs);

    std::string listMenu(const char*, const char*,
                         const std::vector<std::string>&, const char*);

    // TODO: We need to make this template the default one instead of the method
    //       declared later in this code.
    template<size_t N>
    std::string listMenu(const char* title, const char* message,
                               const std::array<std::string_view, N>& items,
                               const char* helpMessage) {

        std::vector<std::string> aux;
        aux.reserve(items.size());
        for (const auto& item : items)
            aux.emplace_back(item);

        return Newt::listMenu(title, message, aux, helpMessage);
    }

    // TODO: Remove this method, it's deprecated
    std::vector<std::string> fieldMenu(const char* title, const char* message,
                                       const std::vector<std::string>& items,
                                       const char* helpMessage);

    // Test function to migrate from std::vector to std::array
    // TODO: Support std::string_view
    template<size_t N>
    std::array<std::pair<std::string, std::variant<std::string, size_t>>, N>
    fieldMenu(const char* title, const char* message,
              const std::array<std::pair<std::string, std::variant<std::string, size_t>>, N>& items,
              const char* helpMessage)
    {
        int returnValue;

        int suggestedWidth = 50;
        int flexUp = 5;
        int flexDown = 5;
        int maxHeightList = 20;

        size_t arraySize = items.size();
        auto fieldEntries = std::make_unique<char*[]>(arraySize + 1);
        auto field = std::make_unique<newtWinEntry[]>(arraySize + 1);

        // If we need to convert a value to a string, we store them here, so we can
        // keep its reference from being destroyed when the next item comes and
        // overwrites the stack data of the previous one
        std::string temporaryStrings;

        // This "for loop" will populate newtWinEntry with the necessary data to be
        // displayed on the interface.
        // Please note that field[i].value is a char** because it's passing data by
        // reference in C style, since the data can be modified by the newt form,
        // it's not an array of char*
        for (size_t i = 0 ; i < arraySize ; i++) {
            field[i].text = const_cast<char*>(items[i].first.c_str());

            if (auto value = std::get_if<size_t>(&items[i].second)) {
                temporaryStrings = std::to_string(*value);
                fieldEntries[i] = const_cast<char*>(
                        temporaryStrings.c_str());
            } else {
                fieldEntries[i] = const_cast<char*>(
                        std::get<std::string>(items[i].second).c_str());
            }

            LOG_TRACE("fieldEntries[{}] = {}", i, fieldEntries[i]);

            // TODO: Check is there's a way to hide &
            field[i].value = &fieldEntries[i];

            // TODO: Fix this hack to enable password fields
            if (items[i].first.find("Password") != std::string::npos ||
                items[i].first.find("password") != std::string::npos)
                field[i].flags = NEWT_FLAG_PASSWORD;
            else
                field[i].flags = 0;
        }

        field[arraySize].text = nullptr;
        field[arraySize].value = nullptr;
        field[arraySize].flags = 0;

        std::array<std::pair<std::string, std::variant<std::string, size_t>>, N> returnArray;

        question:
        returnValue = newtWinEntries(const_cast<char*>(title),
                                     const_cast<char*>(message),
                                     suggestedWidth, flexDown, flexUp,
                                     maxHeightList, field.get(),
                                     const_cast<char*>(MSG_BUTTON_OK),
                                     MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, nullptr);

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
                //        return stdd:string.
                for (size_t i = 0 ; field[i].text ; i++) {
                    returnArray[i] =
                            std::make_pair<std::string, std::variant<std::string, size_t>>(
                                    field[i].text, *field[i].value);
                }

                return returnArray;
            case 2:
                abortInstall();
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

    template <typename T>
    std::vector<std::pair<T, T>>
    fieldMenu(const char* title, const char* message,
                                       const std::vector<std::pair<T, T>>& items,
                                       const char* helpMessage);

    std::vector<std::pair<std::string, std::variant<std::string, size_t>>>
    fieldMenu(const char* title, const char* message,
              const std::vector<std::pair<std::string, std::variant<std::string, size_t>>>& items,
              const char* helpMessage);



    bool yesNoQuestion(const char* title, const char* message,
                       const char* helpMessage);

};

#endif //CLOYSTER_NEWT_H
