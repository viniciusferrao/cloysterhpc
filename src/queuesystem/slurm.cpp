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
    std::filesystem::copy_file("/etc/slurm/slurm.conf.ohpc",
                               "/etc/slurm/slurm.conf");

    runCommand(fmt::format("perl -pi -e "
                           "\"s/ControlMachine=\\S+/ControlMachine={}/\" "
                           "/etc/slurm/slurm.conf",
                           m_cluster.getHeadnode().getFQDN()));
}

void SLURM::enableServer() {
    runCommand("systemctl enable --now munge");
    runCommand("systemctl enable --now slurmctld");
}

void SLURM::startServer() {
    runCommand("systemctl start munge");
    runCommand("systemctl start slurmctld");
}
