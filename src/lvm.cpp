/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/lvm.h>
#include <cloysterhpc/services/log.h>

/*
@TODO -> Preeinstalar os requisitos: boom-boot e rsync.
*/

void LVM::checkUEFIMode()
{
    std::list<std::string> output;
    const std::string checkUEFICommand = "/bin/bash -c \"[ -d /sys/firmware/efi ] && echo UEFI || echo Legacy\"";

    int exitCode = cloyster::runCommand(checkUEFICommand, output, false);

    if (exitCode == 0) {
        auto it = std::find(output.begin(), output.end(), "UEFI");
        if (it != output.end()) {
            LOG_TRACE("LVM: System boot mode: UEFI\n");
        } else {
            LOG_WARN("LVM: System is not booted in UEFI mode.");
        }
    } else {
        LOG_WARN("LVM: Failed to check system boot mode.");
    }
}

void LVM::checkLVMEnabled()
{
    std::list<std::string> output;
    const std::string checkLVMCommand = "vgs --noheadings";
    int exitCode = cloyster::runCommand(checkLVMCommand, output, false);

    if (exitCode == 0) {
        if (!output.empty()) {
            LOG_INFO("LVM is enabled.\n");
        } else {
            throw std::runtime_error("LVM ERROR: LVM is not enabled.");
        }
    } else {
        throw std::runtime_error(
            "LVM ERROR: Failed to check if LVM is enabled.");
    }
}

void LVM::checkThinProvisioning()
{
    std::list<std::string> output;
    const std::string checkThinCommand = "lvs -o+lv_layout --noheadings";
    int exitCode = cloyster::runCommand(checkThinCommand, output, false);

    if (exitCode == 0) {
        auto it = std::find_if(
            output.begin(), output.end(), [](const std::string& line) {
                return line.find("thin") != std::string::npos;
            });

        if (it != output.end()) {
            LOG_TRACE("LVM Thin provisioning is enabled.\n");
        } else {
            LOG_WARN("LVM Thin provisioning is disabled.");
        }
    } else {
        LOG_WARN("Failed to check thin provisioning status.");
    }
}

void LVM::checkEnoughDiskSpaceAvailable()
{
    std::list<std::string> output;
    const std::string checkDiskSpaceCommand
        = "vgs --noheadings -o vg_name,vg_size,vg_free --units G";

    int exitCode = cloyster::runCommand(checkDiskSpaceCommand, output, false);

    if (exitCode == 0 && !output.empty()) {
        try {
            bool allVGsHaveEnoughSpace = true;

            for (const std::string& line : output) {
                LOG_DEBUG("LVM: Check disks space - Raw output line: {}", line);

                // Remove leading/trailing whitespaces
                std::string cleanLine = line;
                cleanLine.erase(cleanLine.begin(), std::find_if(cleanLine.begin(), cleanLine.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
                cleanLine.erase(std::find_if(cleanLine.rbegin(), cleanLine.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(), cleanLine.end());

                if (cleanLine.empty()) {
                    LOG_DEBUG("LVM: Skipping empty or malformed line.");
                    continue;
                }

                // Split the line into vg_name, vg_size, and vg_free
                std::istringstream iss(cleanLine);
                std::string vgName, vgSizeStr, vgFreeStr;
                if (!(iss >> vgName >> vgSizeStr >> vgFreeStr)) {
                    LOG_DEBUG("LVM: Failed to split line into expected fields. Line: {}", cleanLine);
                    throw std::runtime_error("LVM ERROR: Failed to parse volume group information.");
                }

                // Remove 'G' from size and free space strings
                vgSizeStr.erase(std::remove(vgSizeStr.begin(), vgSizeStr.end(), 'G'), vgSizeStr.end());
                vgFreeStr.erase(std::remove(vgFreeStr.begin(), vgFreeStr.end(), 'G'), vgFreeStr.end());

                // Convert vg_size and vg_free from string to double
                double vgSizeGB = std::stod(vgSizeStr);
                double vgFreeGB = std::stod(vgFreeStr);

                // Calculate the percentage of free space
                double freePercentage = (vgFreeGB / vgSizeGB) * 100;

                LOG_TRACE("LVM Volume Group: {}, Total Size: {} GB, Free Space: {} GB, Free Percentage: {:.2f}%",
                    vgName, vgSizeGB, vgFreeGB, freePercentage);

                // Check if the free space is less than 50%
                if (freePercentage < 50.0) {
                    LOG_WARN("LVM Volume Group '{}' does not have enough free space (only {:.2f}% available).",
                        vgName, freePercentage);
                    allVGsHaveEnoughSpace = false;
                }
            }

            if (!allVGsHaveEnoughSpace) {
                throw std::runtime_error("LVM ERROR: Not all LVM volume groups "
                                         "have at least 50% free space.");
            }

            LOG_INFO(
                "LVM: All LVM volume groups have at least 50% free space.");
        } catch (const std::exception& e) {
            throw std::runtime_error(fmt::format(
                "LVM ERROR: Failed to parse disk space information: {}",
                e.what()));
        }
    } else {
        throw std::runtime_error(
            "LVM ERROR: Failed to check available disk space.");
    }
}

void LVM::verifyAvailablePartitions()
{
    std::list<std::string> output;
    const std::string checkPartitionsCommand
        = "lsblk --noheadings --output MOUNTPOINT";
    int exitCode = cloyster::runCommand(checkPartitionsCommand, output, false);

    if (exitCode == 0) {
        for (const auto& line : output) {
            if (line == "/var") {
                m_hasVarPartition = true;
            } else if (line == "/opt") {
                m_hasOptPartition = true;
            } else if (line == "/home") {
                m_hasHomePartition = true;
            }
        }

        if (m_hasHomePartition) {
            LOG_TRACE("LVM: /home is in a separate partition. We can proceed with "
                      "LVM snapshot.");
        } else {
            LOG_WARN(
                "LVM: /home is not a separate partition. This may lead "
                "to issues when rolling back the snapshot.");
        }
    } else {
        throw std::runtime_error(
            "LVM ERROR: Failed to list available partitions.");
    }
}
void LVM::checkLVMAvailability()
{
    LOG_INFO("LVM: Begin of availability check.")
    verifyBootIsNotLVM();
    checkUEFIMode();
    checkLVMEnabled();
    checkThinProvisioning();
    checkEnoughDiskSpaceAvailable();
    verifyAvailablePartitions();
    LOG_INFO("LVM: End of availability check.")
}

bool LVM::snapshotExists(const std::string& snapshotName)
{
    std::list<std::string> output;

    // Construct the command to check if the snapshot exists
    const std::string checkSnapshotCommand = fmt::format("lvs --noheadings -o lv_name {}/{}", m_snapshotVolumeGroup, snapshotName);

    // Run the command
    int exitCode = cloyster::runCommand(checkSnapshotCommand, output, false);

    // If the command succeeded (exitCode 0) and output is not empty, snapshot exists
    if (exitCode == 0 && !output.empty()) {
        // We can further check if the first line matches the snapshot name (sanity check)
        std::string lvName = output.front();

        // Trim leading/trailing whitespaces (just in case)
        lvName.erase(lvName.begin(), std::find_if(lvName.begin(), lvName.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        lvName.erase(std::find_if(lvName.rbegin(), lvName.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), lvName.end());

        // Check if the name matches the expected snapshot name
        if (lvName == snapshotName) {
            LOG_INFO("LVM: Snapshot {} exists.", snapshotName)
            return true;
        }
    }

    // If the command failed or the snapshot was not found
    LOG_INFO("LVM: Snapshot {} does not exist.", snapshotName)
    return false;
}

void LVM::verifyBootIsNotLVM()
{
    std::list<std::string> output;

    const std::string listCommand = "lsblk --noheadings --output MOUNTPOINT,TYPE";
    int exitCodeList = cloyster::runCommand(listCommand, output, false);

    if (exitCodeList != 0) {
        throw std::runtime_error("LVM ERROR: Failed to list mount points.");
    }

    bool isBootPartitionFound = false;

    for (const auto& line : output) {
        std::istringstream iss(line);
        std::string mountPoint, type;
        if (!(iss >> mountPoint >> type)) {
            continue;
        }

        if (mountPoint == "/boot") {
            isBootPartitionFound = true;
            if (type == "lvm") {
                m_hasBootPartition = true;
            }
        }
    }

    if (isBootPartitionFound && !m_hasBootPartition) {
        LOG_INFO("LVM: /boot is mounted and not part of LVM.");
    } else if (m_hasBootPartition) {
        throw std::runtime_error("LVM ERROR: /boot is part of LVM. Check your system configuration.");
    } else {
        LOG_WARN("LVM: /boot is not found or is part of LVM. Check your system configuration.");
    }
}

void LVM::backupBoot()
{
    //@TODO We need a better way to store the backup path
    cloyster::createDirectory("/opt/cloysterhpc/backup");

    //@TODO We need a better way to store the backup path
    const std::string backupCommand
        = "rsync -a /boot/ /opt/cloysterhpc/backup/boot/";

    int exitCode = cloyster::runCommand(backupCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM: /boot backed up successfully.");
    } else {
        throw std::runtime_error("LVM ERROR: Failed to back up /boot.");
    }
}

void LVM::restoreBoot()
{
    //@TODO We need a better way to store the backup path
    const std::string restoreCommand
        = "rsync -a /opt/cloysterhpc/backup/boot/ /boot/";

    int exitCode = cloyster::runCommand(restoreCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM: /boot restored successfully.");
    } else {
        throw std::runtime_error("LVM ERROR: Failed to restore /boot.");
    }
}

void LVM::checkVolumeGroup()
{
    std::list<std::string> output;
    const std::string checkVGCommand = "vgs --noheadings -o vg_name";

    int exitCode = cloyster::runCommand(checkVGCommand, output, false);

    if (exitCode == 0 && !output.empty()) {
        // Assuming we want to get the first volume group found
        std::string vgName = output.front();

        // Trim any leading/trailing whitespaces
        vgName.erase(vgName.begin(), std::find_if(vgName.begin(), vgName.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        vgName.erase(std::find_if(vgName.rbegin(), vgName.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), vgName.end());

        if (vgName.empty()) {
            throw std::runtime_error("LVM ERROR: Volume group name is empty.");
        } else {
            m_snapshotVolumeGroup = vgName;
            LOG_INFO("LVM Volume Group found: {}", vgName);
        }
    } else {
        throw std::runtime_error("LVM ERROR: Failed to find volume group.");
    }
}

void LVM::createSnapshot(const std::string& snapshotName)
{
    if(snapshotExists(snapshotName)) {
        return;
    }

    LOG_INFO("LVM Snapshot creation in progress.");
    checkVolumeGroup();

    const std::string createSnapshotCommand = fmt::format(
        "lvcreate -s -n {} {}/root", snapshotName, m_snapshotVolumeGroup);

    int exitCode = cloyster::runCommand(createSnapshotCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM Snapshot '{}' created successfully.", snapshotName);
    } else {
        throw std::runtime_error(fmt::format(
            "LVM ERROR: Failed to create snapshot '{}'.", snapshotName));
    }
}

void LVM::rollbackSnapshot(const std::string& snapshotName)
{
    if(!snapshotExists(snapshotName)) {
        return;
    }

    LOG_INFO("LVM Snapshot rollback in progress.");
    checkVolumeGroup();

    const std::string rollbackSnapshotCommand = fmt::format(
        "lvconvert --merge {}/{}", m_snapshotVolumeGroup, snapshotName);

    int exitCode = cloyster::runCommand(rollbackSnapshotCommand);

    if (exitCode == 0) {
        LOG_INFO(
            "LVM successfully rolled back to snapshot '{}'.", snapshotName);

        LOG_WARN("Don't forget to reboot after rollback is complete.");
    } else {
        throw std::runtime_error(fmt::format(
            "LVM ERROR: Failed to roll back to snapshot '{}'.", snapshotName));
    }
}

void LVM::removeSnapshot(const std::string& snapshotName)
{
    if(!snapshotExists(snapshotName)) {
        return;
    }

    LOG_INFO("LVM Snapshot removal in progress.");
    checkVolumeGroup();

    std::list<std::string> output;
    const std::string checkSnapshotCommand = fmt::format("lvs --noheadings -o lv_attr {}/{}", m_snapshotVolumeGroup, snapshotName);

    int exitCode = cloyster::runCommand(checkSnapshotCommand, output, false);

    if (exitCode == 0 && !output.empty()) {
        std::string lvAttr = output.front();

        // Check if the snapshot is merged (state will be inactive or missing)
        if (lvAttr.find("M") != std::string::npos) {
            LOG_WARN("LVM Snapshot '{}' is already merged and cannot be removed.", snapshotName);
        } else {
            const std::string removeSnapshotCommand = fmt::format("lvremove {}/{}", m_snapshotVolumeGroup, snapshotName);
            exitCode = cloyster::runCommand(removeSnapshotCommand);

            if (exitCode == 0) {
                LOG_INFO("LVM Snapshot '{}' removed successfully.", snapshotName);
            } else {
                throw std::runtime_error(fmt::format("LVM ERROR: Failed to remove snapshot '{}'.", snapshotName));
            }
        }
    } else {
        throw std::runtime_error("LVM ERROR: Failed to check snapshot status.");
    }
}

void LVM::createSnapshotWithBootBackup(const std::string& snapshotName)
{
    backupBoot();
    createSnapshot(snapshotName);
}

void LVM::rollbackSnapshotWithBootRestore(const std::string& snapshotName)
{
    rollbackSnapshot(snapshotName);
    restoreBoot();
}
