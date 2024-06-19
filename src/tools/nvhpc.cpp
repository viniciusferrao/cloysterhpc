/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/tools/nvhpc.h"
#include "cloysterhpc/functions.h"

void NVhpc::install() { cloyster::runCommand("dnf -y install nvhpc-24.3"); }