//
// Created by Vinícius Ferrão on 11/02/22.
//

#include "queuesystem.h"

void QueueSystem::setKind(Kind kind) {
    m_kind = kind;
}

QueueSystem::Kind QueueSystem::getKind(void) {
    return m_kind;
}

void QueueSystem::setDefaultQueue(std::string_view defaultQueue) {
    m_defaultQueue = defaultQueue;
}

std::string_view QueueSystem::getDefaultQueue(void) {
    return m_defaultQueue;
}
