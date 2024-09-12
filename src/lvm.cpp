/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/functions.h"
#include "cloysterhpc/services/log.h"

#include <cloysterhpc/lvm.h>

/*
@TODO -> Preeinstalar os requisitos: boom-boot e rsync.
*/

void LVM::checkUEFIMode()
{
    std::list<std::string> output;
    const std::string checkUEFICommand
        = "[ -d /sys/firmware/efi ] && echo UEFI || echo Legacy";
    int exitCode = cloyster::runCommand(checkUEFICommand, output, false);

    if (exitCode == 0) {
        auto it = std::find(output.begin(), output.end(), "UEFI");
        if (it != output.end()) {
            LOG_TRACE("System boot mode: UEFI\n");
        } else {
            LOG_WARN("System is not booted in UEFI mode.");
        }
    } else {
        LOG_WARN("Failed to check system boot mode.");
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
        throw std::runtime_error("LVM ERROR: Failed to check if LVM is enabled.");
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
    const std::string checkDiskSpaceCommand = "vgs --noheadings -o vg_name,vg_size,vg_free --units G";
    int exitCode = cloyster::runCommand(checkDiskSpaceCommand, output, false);

    if (exitCode == 0 && !output.empty()) {
        try {
            bool allVGsHaveEnoughSpace = true;

            for (const std::string& line : output) {
                // Remove leading/trailing whitespaces
                std::string cleanLine = line;
                cleanLine.erase(remove_if(cleanLine.begin(), cleanLine.end(), isspace), cleanLine.end());

                // Split the line into vg_name, vg_size, and vg_free
                std::istringstream iss(cleanLine);
                std::string vgName, vgSizeStr, vgFreeStr;
                if (!(iss >> vgName >> vgSizeStr >> vgFreeStr)) {
                    throw std::runtime_error("LVM ERROR: Failed to parse volume group information.");
                }

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
                throw std::runtime_error("LVM ERROR: Not all LVM volume groups have at least 50% free space.");
            }

            LOG_INFO("LVM: All LVM volume groups have at least 50% free space.");
        } catch (const std::exception& e) {
            throw std::runtime_error("LVM ERROR: Failed to parse disk space information.");
        }
    } else {
        throw std::runtime_error("LVM ERROR: Failed to check available disk space.");
    }
}


void LVM::verifyAvailablePartitions()
{
    std::list<std::string> output;
    const std::string checkPartitionsCommand = "lsblk --noheadings --output MOUNTPOINT";
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
            LOG_TRACE("/home is in a separate partition. We can proceed with LVM snapshot.");
        } else {
            throw std::runtime_error("LVM ERROR: /home is not a separate partition. This may lead to issues when rolling back the snapshot.");
        }
    } else {
        throw std::runtime_error("LVM ERROR: Failed to list available partitions.");
    }
}

void LVM::verifyBootIsNotLVM()
{
    std::list<std::string> output;
    const std::string checkBootCommand = "lsblk --noheadings --output MOUNTPOINT";

    int exitCode = cloyster::runCommand(checkBootCommand, output, false);

    if (exitCode == 0 && !output.empty()) {
        for (const auto& line : output) {
            std::string cleanLine = line;
            cleanLine.erase(remove_if(cleanLine.begin(), cleanLine.end(), isspace), cleanLine.end());

            if (cleanLine == "/boot") {
                m_hasBootPartition = true;
                break;
            }
        }

        if (m_hasBootPartition) {
            LOG_TRACE("/boot is not part of any LVM volume group.");
        } else {
            throw std::runtime_error("LVM ERROR: /boot is not found. Check your system configuration.");
        }
    } else {
        throw std::runtime_error("LVM ERROR: Failed to check if /boot is part of LVM.");
    }
}

void LVM::backupBoot()
{
    //@TODO We need a better way to store the backup path
    const std::string backupCommand = "rsync -a /boot/ /opt/opencattus/backup/boot/";

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
    const std::string restoreCommand = "rsync -a /opt/opencattus/backup/boot/ /boot/";

    int exitCode = cloyster::runCommand(restoreCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM: /boot restored successfully.");
    } else {
        throw std::runtime_error("LVM ERROR: Failed to restore /boot.");
    }
}


void LVM::createSnapshot(const std::string& snapshotName)
{
    const std::string createSnapshotCommand = fmt::format(
        "lvcreate -s -n {} {}/root", snapshotName, m_snapshotVolumeGroup);

    int exitCode = cloyster::runCommand(createSnapshotCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM Snapshot '{}' created successfully.", snapshotName);
    } else {
        throw std::runtime_error(fmt::format("LVM ERROR: Failed to create snapshot '{}'.", snapshotName));
    }
}

void LVM::rollbackSnapshot(const std::string& snapshotName)
{
    const std::string rollbackSnapshotCommand = fmt::format(
        "lvconvert --merge {}/{}", m_snapshotVolumeGroup, snapshotName);

    int exitCode = cloyster::runCommand(rollbackSnapshotCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM successfully rolled back to snapshot '{}'.", snapshotName);
    } else {
        throw std::runtime_error(fmt::format("LVM ERROR: Failed to roll back to snapshot '{}'.", snapshotName));
    }
}

void LVM::removeSnapshot(const std::string& snapshotName)
{
    const std::string removeSnapshotCommand = fmt::format(
        "lvremove {}/{}", m_snapshotVolumeGroup, snapshotName);

    int exitCode = cloyster::runCommand(removeSnapshotCommand);

    if (exitCode == 0) {
        LOG_INFO("LVM Snapshot '{}' removed successfully.", snapshotName);
    } else {
        throw std::runtime_error(fmt::format("LVM ERROR: Failed to remove snapshot '{}'.", snapshotName));
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

LVM::LVM()
{
    verifyBootIsNotLVM();
    checkUEFIMode();
    checkLVMEnabled();
    checkThinProvisioning();
    checkSystemSupport();
    checkEnoughDiskSpaceAvailable();
    verifyAvailablePartitions();
}

#ifdef BUILD_TESTING
#include <cloysterhpc/tests.h>

TEST_SUITE("Test LVM")
{

    TEST_CASE("Check if system supports LVM snapshots")
    {

    }
}
#endif