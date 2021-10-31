//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTVIEWBASE_H
#define CLOYSTER_NEWTVIEWBASE_H

#include <vector>
#include <string>
#include <iostream>
#include <newt.h>
#include "view.h"
#include "../messages.h" /* Legacy constexpr */

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class NewtViewBase : public View {
protected:
    void abortInstall () override;
    void helpMessage (const char*) override;
    bool hasEmptyField (const struct newtWinEntry*);

#ifdef _DEBUG_
    void debugEntries(const struct newtWinEntry*);
#endif

public:
    NewtViewBase();
    ~NewtViewBase() override;

#if 0
    virtual std::string timezoneSelection
            (const std::vector<std::string>&) = 0;
    virtual std::string localeSelection
            (const std::vector<std::string>&) = 0;
#endif
};

#endif //CLOYSTER_NEWTVIEWBASE_H
