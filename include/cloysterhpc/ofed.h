/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_OFED_H_
#define CLOYSTERHPC_OFED_H_

#include <cloysterhpc/functions.h>

class OFED {
public:
    enum class Kind { Inbox, Mellanox, Oracle };

private:
    Kind m_kind { Kind::Inbox };
    void patchMellanox() const;

public:
    OFED() = default;
    explicit OFED(Kind kind);

    void setKind(Kind kind);
    [[nodiscard]] Kind getKind() const;

    void install() const;
};

#endif // CLOYSTERHPC_OFED_H_
