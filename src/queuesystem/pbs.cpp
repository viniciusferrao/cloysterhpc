//
// Created by Vinícius Ferrão on 11/02/22.
//

#include "pbs.h"

PBS::PBS() {
    setKind(QueueSystem::Kind::PBS);
}

void PBS::setExecutionPlace(ExecutionPlace executionPlace) {
    m_executionPlace = executionPlace;
}

PBS::ExecutionPlace PBS::getExecutionPlace(void) {
    return m_executionPlace;
}
