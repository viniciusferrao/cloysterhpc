/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/tools/nvhpc.h>

void NVhpc::install()
{
    LOG_TRACE("Installing NVIDIA HPC SDK");
    cloyster::runCommand("dnf -y install nvhpc-24.3");
}

void NVhpc::configure() {};
