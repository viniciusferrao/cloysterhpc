/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CPU_H_
#define CLOYSTERHPC_CPU_H_

#include <cstddef>
#include <stdexcept>

class CPU {
private:
    std::size_t m_sockets;
    std::size_t m_cores;
    std::size_t m_threads;
    std::size_t m_coresPerSocket;
    std::size_t m_threadsPerCore;

    void checkTopology() const;

public:
    CPU();
    CPU(std::size_t sockets, std::size_t coresPerSocket,
        std::size_t threadsPerCore);
    CPU(std::size_t sockets, std::size_t cores, std::size_t threads,
        std::size_t coresPerSocket, std::size_t threadsPerCore);

    [[nodiscard]] size_t getSockets() const;
    [[nodiscard]] size_t getCores() const;
    [[nodiscard]] size_t getThreads() const;
    [[nodiscard]] size_t getCoresPerSocket() const;
    [[nodiscard]] size_t getThreadsPerCore() const;

    void setSockets(size_t sockets);
    void setCores(size_t cores);
    void setThreads(size_t threads);
    void setCoresPerSocket(size_t coresPerSocket);
    void setThreadsPerCore(size_t threadsPerCore);
};

#endif // CLOYSTERHPC_CPU_H_
