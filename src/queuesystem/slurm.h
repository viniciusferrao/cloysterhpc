//
// Created by Vinícius Ferrão on 11/02/22.
//

#ifndef CLOYSTERHPC_SLURM_H
#define CLOYSTERHPC_SLURM_H

#include "queuesystem.h"

class SLURM : public QueueSystem {
private:
    bool m_accounting{false};

public:
    explicit SLURM(const Cluster& cluster);

    void installServer();
    void configureServer();
    void enableServer();
    void startServer();
};

#endif //CLOYSTERHPC_SLURM_H
