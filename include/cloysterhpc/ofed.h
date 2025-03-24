/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_OFED_H_
#define CLOYSTERHPC_OFED_H_

#include <string>
#include <utility>

#include <cloysterhpc/services/repos.h>


/**
 * @class OFED
 * @brief A class representing the OpenFabrics Enterprise Distribution (OFED).
 *
 * OFED is a software package that provides a set of high-performance
 * communication libraries and drivers for use with InfiniBand and Ethernet
 * hardware.
 */
class OFED {
public:
    /**
     * @enum Kind
     * @brief Enumeration representing different kinds of OFED installations.
     */
    enum class Kind { Inbox, Mellanox, Oracle };

private:
    Kind m_kind { Kind::Inbox };
    std::string m_version;

public:
    OFED() = default;
    OFED(Kind kind, std::string version)
    : m_kind(kind)
    , m_version(std::move(version))
    {}

    void setKind(Kind kind);
    [[nodiscard]] Kind getKind() const;

    void setVersion(std::string&& value) { m_version = std::move(value); }
    [[nodiscard]] std::string getVersion() const { return m_version; }

    /**
     * @brief Installs the OFED software package.
     *
     * This method installs the appropriate OFED components based on the
     * specified kind.
     */
    void install() const;

    /**
     * @brief Returns true if OFED is already installed
     */
    [[nodiscard]] bool installed() const;
};

#endif // CLOYSTERHPC_OFED_H_
