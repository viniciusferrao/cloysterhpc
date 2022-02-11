//
// Created by Vinícius Ferrão on 11/02/22.
//

#include "pbs.h"

void PBS::setExecutionPlace(ExecutionPlace executionPlace) {
    m_executionPlace = executionPlace;
}

PBS::ExecutionPlace PBS::getExecutionPlace(void) {
    return m_executionPlace;
}
