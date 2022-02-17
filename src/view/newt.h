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

//    template<size_t N>
//    std::string listMenu(const char*, const char*,
//                         const std::array<std::string_view, N>&, const char*);

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
    template <typename T>
    std::vector<std::pair<T, T>>
    fieldMenu(const char* title, const char* message,
                                       const std::vector<std::pair<T, T>>& items,
                                       const char* helpMessage);

    std::vector<std::pair<std::string, std::variant<std::string, unsigned>>>
    fieldMenu(const char* title, const char* message,
              const std::vector<std::pair<std::string, std::variant<std::string, unsigned>>>& items,
              const char* helpMessage);



    bool yesNoQuestion(const char* title, const char* message,
                       const char* helpMessage);

};

#endif //CLOYSTER_NEWT_H
