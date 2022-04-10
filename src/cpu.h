//
// Created by Vinícius Ferrão on 09/04/22.
//

#ifndef CLOYSTERHPC_CPU_H
#define CLOYSTERHPC_CPU_H

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
};

#endif //CLOYSTERHPC_CPU_H
