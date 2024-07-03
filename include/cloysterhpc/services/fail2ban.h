/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_FAIL2BAN_H_
#define CLOYSTERHPC_FAIL2BAN_H_

#include "IService.h"
class fail2ban : public IService {
public:
    void install() override;
    void configure() override;
    void enable() override;
    void disable() override;
    void start() override;
    void stop() override;
};

#endif // CLOYSTERHPC_FAIL2BAN_H_
