/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
*/

#include "cloysterhpc/hardware.h"
#include <fmt/format.h>

std::vector<hwinfo::CPU> Hardware::getCPU() const {
    return m_cpu;
}

std::vector<hwinfo::Disk> Hardware::getDisk() const {
    return m_disk;
}

std::vector<hwinfo::GPU> Hardware::getGPU() const {
    return m_gpu;
}

hwinfo::MainBoard Hardware::getMainBoard() const {
    return m_mobo;
}

hwinfo::Memory Hardware::getMemory() const {
    return m_memory;
}

hwinfo::OS Hardware::getOS() const {
    return m_os;
}

Hardware::Hardware() {
    m_cpu = hwinfo::getAllCPUs();
    m_disk = hwinfo::getAllDisks();
    m_gpu = hwinfo::getAllGPUs();
}

void Hardware::printOverview() {
    fmt::println("Operational System:\n");
    fmt::println("     Operational System Overview:\n");
    fmt::println("        System Version: {}", m_os.name());
    fmt::println("        Kernel Version: {}", m_os.kernel());
    fmt::println("");

    printHardwareInfo();
    printMotherboardInfo();
    printDiskInfo();
    printGPUInfo();
}

void Hardware::printHardwareInfo() {
    fmt::println("Hardware:\n");
    fmt::println("     Hardware Overview:\n");
    fmt::println("        Chip: {}", m_cpu[0].modelName());
    fmt::println("        Physical Cores: {}", m_cpu[0].numPhysicalCores());
    fmt::println("        Logical Cores: {}", m_cpu[0].numLogicalCores());
    fmt::println("        Vendor: {}", m_cpu[0].vendor());
    fmt::println("        Memory: {} GB ({} bytes)",
                 convertByteToGB(m_memory.total_Bytes()),
                 m_memory.total_Bytes());
    fmt::println("");
}

void Hardware::printGPUInfo() {
    fmt::println("     GPUs Overview:\n");
    for (auto const& gpu: m_gpu) {
        fmt::println("        {}: ", gpu.name());
        fmt::println("           Total Memory: {} GB ({} bytes)",
                     convertByteToGB(gpu.memory_Bytes()), gpu.memory_Bytes());
        fmt::println("           Total Number of Cores: {}", gpu.num_cores());
    }
    fmt::println("");
}

void Hardware::printDiskInfo() {
    fmt::println("     Disks Overview:\n");
    for (auto const &disk: m_disk) {
        fmt::println("        {}: ", disk.model());
        fmt::println("           Total Size: {} GB ({} bytes)",
                     convertByteToGB(disk.size_Bytes()), disk.size_Bytes());
        fmt::println("           Serial Number: {}", disk.serialNumber());
    }
    fmt::println("");
}

void Hardware::printMotherboardInfo() const {
    fmt::println("     Motherboard Overview:\n");
    fmt::println("        Name: {}", m_mobo.name());
    fmt::println("        Version: {}", m_mobo.version());
    fmt::println("        Vendor: {}", m_mobo.vendor());
    fmt::println("        Serial Number: {}", m_mobo.serialNumber());
    fmt::println("");
}

int64_t Hardware::convertByteToGB(int64_t bytes) {
    return bytes / 1073741824;
    // 1073741824 is the same as static_cast<int64_t>(std::pow(1024,3))
}