/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/services/nvhpc.h"
#include "cloysterhpc/functions.h"

void NVhpc::install() { cloyster::runCommand("dnf -y install nvhpc-24.3"); }

void NVhpc::enable() { }
void NVhpc::disable() { }
void NVhpc::start() { }
void NVhpc::stop() { }