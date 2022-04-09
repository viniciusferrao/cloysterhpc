//
// Created by Vinícius Ferrão on 09/04/22.
//

#include "cpu.h"

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

void CPU::checkTopology() {
    if (!m_sockets || !m_cores || !m_threads || !m_coresPerSocket || !m_threadsPerCore)
        throw std::runtime_error("Invalid number of processors");
    if (m_threads != (m_cores * m_threadsPerCore))
        throw std::runtime_error("Total number of threads is not valid");
    if (m_cores != (m_sockets * m_coresPerSocket))
        throw std::runtime_error("Total number of cores is not valid");
}
