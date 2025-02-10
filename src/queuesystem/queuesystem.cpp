/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/queuesystem/queuesystem.h>
#include <cloysterhpc/runner.h>

template<>
QueueSystem<BaseRunner>::QueueSystem(const Cluster<BaseRunner>& cluster)
    : m_cluster(cluster)
{
}

template<>
void QueueSystem<BaseRunner>::setKind(Kind kind) { m_kind = kind; }

template<>
QueueSystem<BaseRunner>::Kind QueueSystem<BaseRunner>::getKind(void) { return m_kind; }

template<>
void QueueSystem<BaseRunner>::setDefaultQueue(std::string_view defaultQueue)
{
    m_defaultQueue = defaultQueue;
}

template<>
std::string_view QueueSystem<BaseRunner>::getDefaultQueue(void) { return m_defaultQueue; }
