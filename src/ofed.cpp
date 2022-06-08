/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ofed.h"

using cloyster::runCommand;

OFED::OFED(Kind kind) : m_kind(kind) {}

void OFED::setKind(Kind kind) {
    m_kind = kind;
}

OFED::Kind OFED::getKind() const {
    return m_kind;
}

void OFED::install() const {
    switch (m_kind) {
        case OFED::Kind::Inbox:
            runCommand("dnf -y groupinstall \"Infiniband Support\"");

            break;

        case OFED::Kind::Mellanox:
            throw std::logic_error("MLNX OFED is not yet supported");

            break;

        case OFED::Kind::Oracle:
            throw std::logic_error("Oracle RDMA release is not yet supported");

            break;
    }
}
