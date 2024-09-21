/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_FAIL2BAN_H_
#define CLOYSTERHPC_FAIL2BAN_H_

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/IService.h>
#include <cloysterhpc/services/log.h>

class fail2ban : public IService {
private:
    void configure() override;

public:
    void install() override;
    void enable() override;
    void disable() override;
    void start() override;
    void stop() override;
};

#endif // CLOYSTERHPC_FAIL2BAN_H_
