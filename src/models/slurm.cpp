/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/slurm.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/osservice.h>
#include <filesystem>

namespace cloyster::models {
SLURM::SLURM(const Cluster& cluster)
    : QueueSystem(cluster)
{
    setKind(QueueSystem::Kind::SLURM);
}

void SLURM::installServer()
{
    cloyster::Singleton<cloyster::services::IOSService>::get()->install(
        "ohpc-slurm-server");
}

void SLURM::configureServer()
{
    const std::string configurationFile { "/etc/slurm/slurm.conf" };
    cloyster::removeFile(configurationFile);

    // Ensure that the directory exists
    // TODO: This may be made on cloyster::addStringToFile?
    cloyster::createDirectory("/etc/slurm");

    std::vector<std::string> nodes;
    nodes.reserve(m_cluster.getNodes().size());

    for (const auto& node : m_cluster.getNodes())
        nodes.emplace_back(
            fmt::format("NodeName={} Sockets={} CoresPerSocket={} "
                        "ThreadsPerCore={} State=UNKNOWN",
                node.getHostname(), node.getCPU().getSockets(),
                node.getCPU().getCoresPerSocket(),
                node.getCPU().getThreadsPerCore()));

    const auto& conf { fmt::format(
#include "cloysterhpc/tmpl/slurm.conf.tmpl"
        , fmt::arg("clusterName", m_cluster.getName()),
        fmt::arg("controlMachine", m_cluster.getHeadnode().getFQDN()),
        fmt::arg("partitionName", getDefaultQueue()),
        fmt::arg("nodesDeclaration", fmt::join(nodes, "\n"))) };

    cloyster::addStringToFile(configurationFile, conf);
}

void SLURM::enableServer()
{
    auto osservice = cloyster::Singleton<services::IOSService>::get();
    osservice->enableService("munge");
    osservice->enableService("slurmctld");
}

void SLURM::startServer()
{
    auto osservice = cloyster::Singleton<services::IOSService>::get();
    osservice->startService("munge");
    osservice->startService("slurmctld");
}

}
