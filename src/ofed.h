//
// Created by Vinícius Ferrão on 13/04/22.
//

#ifndef CLOYSTERHPC_OFED_H
#define CLOYSTERHPC_OFED_H

#include "functions.h"

class OFED {
public:
    enum class Kind { Inbox, Mellanox, Oracle };

private:
    Kind m_kind{Kind::Inbox};

public:
    OFED() = default;
    explicit OFED(Kind kind);

    void setKind(Kind kind);
    [[nodiscard]] Kind getKind() const;

    void install() const;
};


#endif //CLOYSTERHPC_OFED_H
