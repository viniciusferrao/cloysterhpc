/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ITOOL_H_
#define CLOYSTERHPC_ITOOL_H_

#include "cloysterhpc/functions.h"
#include "cloysterhpc/services/log.h"

class ITool {
protected:
    [[maybe_unused]] virtual void configure() = 0;

public:
    virtual ~ITool() = default;
    virtual void install() = 0;
};

#endif // CLOYSTERHPC_ITOOL_H_
