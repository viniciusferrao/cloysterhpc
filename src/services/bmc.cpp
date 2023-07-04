/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bmc.h"
BMC::BMC(const std::string& address, const std::string& username,
    const std::string& password, int serialPort, int serialSpeed,
    const BMC::kind& kind)
    : m_address(address)
    , m_username(username)
    , m_password(password)
    , m_serialPort(serialPort)
    , m_serialSpeed(serialSpeed)
    , m_kind(kind)
{
}
