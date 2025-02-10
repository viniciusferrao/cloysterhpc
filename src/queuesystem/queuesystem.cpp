/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/queuesystem/queuesystem.h>
#include <cloysterhpc/runner.h>

QueueSystem::QueueSystem(const Cluster& cluster)
    : m_cluster(cluster)
{
}

void QueueSystem::setKind(Kind kind) { m_kind = kind; }

QueueSystem::Kind QueueSystem::getKind(void) { return m_kind; }

void QueueSystem::setDefaultQueue(std::string_view defaultQueue)
{
    m_defaultQueue = defaultQueue;
}

std::string_view QueueSystem::getDefaultQueue(void) { return m_defaultQueue; }
