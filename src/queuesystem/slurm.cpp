//
// Created by Vinícius Ferrão on 11/02/22.
//

#include "slurm.h"
#include "../cluster.h"

using cloyster::runCommand;

SLURM::SLURM(const Cluster& cluster)
    : QueueSystem(cluster)
{
    setKind(QueueSystem::Kind::SLURM);
}

void SLURM::installServer() {
    runCommand("dnf -y install ohpc-slurm-server");
}

void SLURM::configureServer() {
    cloyster::removeFile("/etc/slurm/slurm.conf");

    std::string nodesDeclaration{};

    for (const auto& node : m_cluster.getNodes())
        nodesDeclaration += fmt::format(
            "NodeName={} Sockets={} CoresPerSocket={} ThreadsPerCore={} State=UNKNOWN\n"
            , node.getHostname()
            , node.getCPU().getSockets()
            , node.getCPU().getCoresPerSocket()
            , node.getCPU().getThreadsPerCore()
        );

    std::string conf = fmt::format(
            #include "../tmpl/slurm.conf.tmpl"
            , fmt::arg("clusterName", m_cluster.getName())
            , fmt::arg("controlMachine", m_cluster.getHeadnode().getFQDN())
            , fmt::arg("partitionName", getDefaultQueue())
            , fmt::arg("nodesDeclaration", nodesDeclaration)
    );

    cloyster::addStringToFile("/etc/slurm/slurm.conf", conf);

}

void SLURM::enableServer() {
    runCommand("systemctl enable --now munge");
    runCommand("systemctl enable --now slurmctld");
}

void SLURM::startServer() {
    runCommand("systemctl start munge");
    runCommand("systemctl start slurmctld");
}
