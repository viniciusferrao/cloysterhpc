//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTFIELDMENU_H
#define CLOYSTER_NEWTFIELDMENU_H

#include "newt.h"

class NewtFieldMenu : public Newt {
private:
    std::vector<std::string> draw (const char*, const char*,
                                   const std::vector<std::string>&,
                                   const char*);
};

#endif //CLOYSTER_NEWTFIELDMENU_H
