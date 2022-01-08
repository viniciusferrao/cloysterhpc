//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTLISTMENU_H
#define CLOYSTER_NEWTLISTMENU_H

#include "newt.h"

class NewtListMenu : public Newt {
public:
    std::string draw (const char*, const char*, const std::vector<std::string>&,
                      const char*);
};

#endif //CLOYSTER_NEWTLISTMENU_H
