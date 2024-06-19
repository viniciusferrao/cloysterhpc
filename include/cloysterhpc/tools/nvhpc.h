/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NVHPC_H_
#define CLOYSTERHPC_NVHPC_H_

#include <cloysterhpc/tools/ITool.h>

class NVhpc : public ITool {
private:
    void configure() override;

public:
    void install() override;
};

#endif // CLOYSTERHPC_NVHPC_H_
