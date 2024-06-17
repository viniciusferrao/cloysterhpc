/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/services/nvhpc.h"
#include "cloysterhpc/functions.h"

//@TODO add repo
//$ sudo dnf config-manager --add-repo
//https://developer.download.nvidia.com/hpc-sdk/rhel/nvhpc.repo
void NVhpc::install() { cloyster::runCommand("dnf -y install nvhpc-24.3"); }

void NVhpc::enable() { }
void NVhpc::disable() { }
void NVhpc::start() { }
void NVhpc::stop() { }