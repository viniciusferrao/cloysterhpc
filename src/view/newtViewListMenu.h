//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTVIEWLISTMENU_H
#define CLOYSTER_NEWTVIEWLISTMENU_H

#include "newtViewBase.h"

class NewtViewListMenu : public NewtViewBase {
private:
    std::string draw (const char*, const char*, const std::vector<std::string>&,
                      const char*);
};

#endif //CLOYSTER_NEWTVIEWLISTMENU_H
