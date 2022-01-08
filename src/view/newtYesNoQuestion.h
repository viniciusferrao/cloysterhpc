//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_NEWTYESNOQUESTION_H
#define CLOYSTER_NEWTYESNOQUESTION_H

#include "newt.h"

class NewtYesNoQuestion : public Newt {
private:
    bool draw (const char*, const char*, const char*);
};

#endif //CLOYSTER_NEWTYESNOQUESTION_H
