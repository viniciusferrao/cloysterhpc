/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_BMC_H_
#define CLOYSTERHPC_BMC_H_

#include <string>

struct BMC {
    enum class kind { IPMI };

    std::string m_address;
    std::string m_username;
    std::string m_password;
    int m_serialPort { 0 };
    int m_serialSpeed { 115200 };
    kind m_kind = kind::IPMI;

    BMC(const std::string& address, const std::string& username,
        const std::string& password, int serialPort, int serialSpeed,
        const kind& kind);
};

#endif // CLOYSTERHPC_BMC_H_
