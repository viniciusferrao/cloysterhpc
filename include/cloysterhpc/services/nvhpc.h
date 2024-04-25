/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NVHPC_H_
#define CLOYSTERHPC_NVHPC_H_

#include "IService.h"

class NVhpc : public IService {
private:
public:
    void install();
    void enable() override;
    void disable() override;
    void start() override;
    void stop() override;
};

#endif // CLOYSTERHPC_NVHPC_H_
