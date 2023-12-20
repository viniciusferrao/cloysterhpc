/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
*/

#ifndef CLOYSTERHPC_HARDWARE_H_
#define CLOYSTERHPC_HARDWARE_H_

#include <hwinfo/hwinfo.h>

class Hardware {
private:
    std::vector<hwinfo::CPU> m_cpu;
    std::vector<hwinfo::Disk> m_disk;
    std::vector<hwinfo::GPU> m_gpu;
    hwinfo::MainBoard m_mobo;
    hwinfo::Memory m_memory;
    hwinfo::OS m_os;

    static int64_t convertByteToGB(int64_t bytes);

    void printHardwareInfo();

    void printMotherboardInfo() const;

    void printDiskInfo();

    void printGPUInfo();

public:
    Hardware();

    [[nodiscard]] std::vector<hwinfo::CPU> getCPU() const;

    [[nodiscard]] std::vector<hwinfo::Disk> getDisk() const;

    [[nodiscard]] std::vector<hwinfo::GPU> getGPU() const;

    [[nodiscard]] hwinfo::MainBoard getMainBoard() const;

    [[nodiscard]] hwinfo::Memory getMemory() const;

    [[nodiscard]] hwinfo::OS getOS() const;

    void printOverview();
};

#endif //CLOYSTERHPC_HARDWARE_H_
