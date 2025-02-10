/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/cluster.h>
#include <cloysterhpc/queuesystem/slurm.h>
#include <cloysterhpc/services/log.h>
#include <filesystem>

using cloyster::runCommand;

SLURM::SLURM(const Cluster<BaseRunner>& cluster)
    : QueueSystem(cluster)
{
    setKind(QueueSystem::Kind::SLURM);
}

void SLURM::installServer() { runCommand("dnf -y install ohpc-slurm-server"); }

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
    runCommand("systemctl enable --now munge");
    runCommand("systemctl enable --now slurmctld");
}

void SLURM::startServer()
{
    runCommand("systemctl start munge");
    runCommand("systemctl start slurmctld");
}
