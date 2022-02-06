//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWT_H
#define CLOYSTER_NEWT_H

#include <vector>
#include <string>
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

#ifdef _DEBUG_
    void debugEntries(const struct newtWinEntry*);
#endif

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
    std::vector<std::string> fieldMenu(const char* title, const char* message,
                                       const std::vector<std::string>& items,
                                       const char* helpMessage);
    bool yesNoQuestion(const char* title, const char* message,
                       const char* helpMessage);

};

#endif //CLOYSTER_NEWT_H
