/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/queuesystem/pbs.h>

PBS::PBS(const Cluster<BaseRunner>& cluster)
    : QueueSystem(cluster)
{
    setKind(QueueSystem::Kind::PBS);
}

void PBS::setExecutionPlace(ExecutionPlace executionPlace)
{
    m_executionPlace = executionPlace;
}

PBS::ExecutionPlace PBS::getExecutionPlace(void) { return m_executionPlace; }
