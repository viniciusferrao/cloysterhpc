/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/models/headnode.h>
#include <cloysterhpc/services/log.h>

#include <algorithm> /* std::remove */
#include <fstream>
#include <iostream>
#include <map> /* std::map */
#include <regex>

#include <sys/utsname.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <string>

namespace cloyster::models {

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
    struct utsname system {};
    uname(&system);

    std::string fqdn = system.nodename;
    std::vector<std::string> hostparts;

    boost::split(hostparts, std::string { fqdn }, boost::is_any_of("."));

    std::string hostname = fqdn.empty() ? "localhost" : hostparts[0];
    bool hasDomain = hostparts.size() > 1 && !hostparts[1].empty()
        && hostparts[1] != "(none)";

    setHostname(hostname);
    setFQDN(hasDomain ? fqdn : hostname + ".localdomain");
}

Headnode::BootTarget Headnode::getBootTarget() const { return m_bootTarget; }

void Headnode::setBootTarget(Headnode::BootTarget bootTarget)
{
    m_bootTarget = bootTarget;
}

}; 
