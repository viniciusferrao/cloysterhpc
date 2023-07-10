/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/headnode.h>
#include <cloysterhpc/services/log.h>

#include <algorithm> /* std::remove */
#include <fstream>
#include <iostream>
#include <map> /* std::map */
#include <regex>

#include <sys/utsname.h>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

/* The constructor should discover everything we need from the machine that is
 * running the software. We always consider that the software runs from the
 * server that will become the cluster headnode.
 */
// Headnode::Headnode () = default;

Headnode::Headnode()
    : m_bootTarget(BootTarget::Text)
{
    discoverNames();
}

void Headnode::discoverNames()
{
    struct utsname system { };
    uname(&system);

    std::string hostname = system.nodename;
    setHostname(hostname.substr(0, hostname.find('.')));
    setFQDN(hostname);
}

Headnode::BootTarget Headnode::getBootTarget() const { return m_bootTarget; }

void Headnode::setBootTarget(Headnode::BootTarget bootTarget)
{
    m_bootTarget = bootTarget;
}
