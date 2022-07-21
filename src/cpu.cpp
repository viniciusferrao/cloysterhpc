/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cpu.h"

CPU::CPU()
{
    // TODO: Implement a CPU detection algorithm
}

CPU::CPU(
    std::size_t sockets, std::size_t coresPerSocket, std::size_t threadsPerCore)
    : CPU(sockets, coresPerSocket * sockets,
        threadsPerCore * coresPerSocket * sockets, coresPerSocket,
        threadsPerCore)
{
}

CPU::CPU(std::size_t sockets, std::size_t cores, std::size_t threads,
    std::size_t coresPerSocket, std::size_t threadsPerCore)
    : m_sockets(sockets)
    , m_cores(cores)
    , m_threads(threads)
    , m_coresPerSocket(coresPerSocket)
    , m_threadsPerCore(threadsPerCore)
{
    checkTopology();
}

void CPU::checkTopology() const
{
    if (!m_sockets || !m_cores || !m_threads || !m_coresPerSocket
        || !m_threadsPerCore)
        throw std::runtime_error("Invalid number of processors");
    if (m_threads != (m_cores * m_threadsPerCore))
        throw std::runtime_error("Total number of threads is not valid");
    if (m_cores != (m_sockets * m_coresPerSocket))
        throw std::runtime_error("Total number of cores is not valid");
}

size_t CPU::getSockets() const { return m_sockets; }

size_t CPU::getCores() const { return m_cores; }

size_t CPU::getThreads() const { return m_threads; }

size_t CPU::getCoresPerSocket() const { return m_coresPerSocket; }

size_t CPU::getThreadsPerCore() const { return m_threadsPerCore; }
