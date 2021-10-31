//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTVIEWFIELDMENU_H
#define CLOYSTER_NEWTVIEWFIELDMENU_H

#include "newtViewBase.h"

class NewtViewFieldMenu : public NewtViewBase {
private:
    std::vector<std::string> draw (const char*, const char*,
                                   const std::vector<std::string>&,
                                   const char*);
};

#endif //CLOYSTER_NEWTVIEWFIELDMENU_H
