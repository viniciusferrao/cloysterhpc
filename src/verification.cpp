//
// Created by Lucas B. Gracioso on 15/07/22.
//

#include "verification.h"

#include <iostream>
#include <atomic>

namespace cloyster {

    void checkEffectiveUserId() {
        if (geteuid() != 0) {
            throw std::runtime_error("This program must run with root privileges");
        }
    }

}

