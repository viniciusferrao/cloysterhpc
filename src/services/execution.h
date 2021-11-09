//
// Created by Vinícius Ferrão on 31/10/21.
//

#ifndef CLOYSTER_EXECUTION_H
#define CLOYSTER_EXECUTION_H

#include <string>

#include "../cluster.h"

class Execution {
public:
    virtual ~Execution() = default;

    //virtual void runCommand(const std::string&) = 0;
    virtual void install(const std::unique_ptr<Cluster>&) = 0;
};

#endif //CLOYSTER_EXECUTION_H
