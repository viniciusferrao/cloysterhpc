//
// Created by Vinícius Ferrão on 31/10/21.
//

#ifndef CLOYSTER_SHELL_H
#define CLOYSTER_SHELL_H

#include "execution.h"

class Shell : public Execution {
    void runCommand(const std::string&) override;
};

#endif //CLOYSTER_SHELL_H
