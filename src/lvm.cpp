/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/lvm.h>
#include <cloysterhpc/services/log.h>

/*
@TODO -> Pre-install boom-boot and rsync.
*/

//@TODO Move checkUEFIMode to a healthcheck class
bool LVM::isUEFIModeEnabled()
{
    if (std::filesystem::exists("/sys/firmware/efi")) {
        LOG_TRACE("LVM: System boot mode: UEFI\n");
        return true;
    }

    LOG_WARN("LVM: System is not booted in UEFI mode.");
    return false;
}

bool LVM::isLVMEnabled()
{
    std::list<std::string> output;
    const std::string checkLVMCommand = "vgs --noheadings";
    int exitCode = cloyster::runCommand(checkLVMCommand, output);

    if (exitCode == 0) {
        if (!output.empty()) {
            LOG_INFO("LVM is enabled.\n");
            return true;
        } else {
            LOG_WARN("LVM ERROR: LVM is not enabled.");
            return false;
        }
    }

    throw std::runtime_error("LVM ERROR: Failed to check if LVM is enabled.");
}

bool LVM::isRootLVMEnabled()
{
    std::list<std::string> output;
    const std::string checkRootLVMCommand = "findmnt -n -o SOURCE /";
    int exitCode = cloyster::runCommand(checkRootLVMCommand, output);

    if (exitCode != 0) {
        throw std::runtime_error(
            "LVM ERROR: Failed to check if root filesystem is on LVM.");
    }

    if (output.empty()) {
        throw std::runtime_error(
            "LVM ERROR: Root filesystem check returned no output.");
    }

    const std::string& rootDevice = output.front();

    // Check if the root device is managed by LVM
    // (appears under /dev/mapper/ or /dev/<volume-group>/)
    // When LVM creates a logical volume, it also creates a
    // device file under /dev/mapper/<volume-group>-<logical-volume> for each
    // logical volume, which is a symlink to
    // /dev/<volume-group>/<logical-volume>
    if (rootDevice.find("/dev/mapper/") != std::string::npos
        || rootDevice.find(fmt::format("/dev/{}", m_snapshotVolumeGroup))
            != std::string::npos) {
        LOG_INFO("LVM: Root filesystem is on LVM.\n");
        return true;
    }

    LOG_WARN("LVM: Root filesystem is not on LVM.\n");
    return false;
}

bool LVM::isThinProvisioningEnabled()
{
    std::list<std::string> output;
    const std::string checkThinCommand = "lvs -o+lv_layout --noheadings";
    int exitCode = cloyster::runCommand(checkThinCommand, output);

    if (exitCode != 0) {
        throw std::runtime_error(
            "LVM ERROR: Failed to check thin provisioning status.");
    }

    if (output.empty()) {
        throw std::runtime_error(
            "LVM ERROR: No output returned from thin provisioning check.");
    }

    // Check if any of the volumes have "thin" in the layout
    auto it = std::find_if(
        output.begin(), output.end(), [](const std::string& line) {
            return line.find("thin") != std::string::npos;
        });

    if (it != output.end()) {
        LOG_INFO("LVM: Thin provisioning is enabled.\n");
        return true;
    }

    LOG_WARN("LVM: Thin provisioning is disabled.\n");
    return false;
}

bool LVM::isRootThinProvisioningEnabled()
{
    std::list<std::string> output;

    const std::string checkRootCommand = "findmnt -n -o SOURCE /";
    int exitCode = cloyster::runCommand(checkRootCommand, output);

    if (exitCode != 0) {
        throw std::runtime_error("LVM ERROR: Failed to check the root device.");
    }

    if (output.empty()) {
        throw std::runtime_error(
            "LVM ERROR: No output when checking the root device.");
    }

    const std::string& rootDevice = output.front();

    // Step 2: Use `lvs` to check if the root device is thin-provisioned
    std::list<std::string> lvsOutput;
    const std::string checkThinCommand
        = fmt::format("lvs -o+lv_layout --noheadings {}", rootDevice);
    exitCode = cloyster::runCommand(checkThinCommand, lvsOutput);

    if (exitCode != 0) {
        throw std::runtime_error("LVM ERROR: Failed to check thin provisioning "
                                 "for the root device.");
    }

    if (lvsOutput.empty()) {
        throw std::runtime_error("LVM ERROR: No output when checking thin "
                                 "provisioning for the root device.");
    }

    // Step 3: Look for "thin" in the layout of the root logical volume
    auto it = std::find_if(
        lvsOutput.begin(), lvsOutput.end(), [](const std::string& line) {
            return line.find("thin") != std::string::npos;
        });

    if (it != lvsOutput.end()) {
        LOG_INFO(
            "LVM: Root filesystem is on LVM with Thin provisioning enabled.\n");
        return true;
    } else {
        LOG_WARN("LVM: Root filesystem is on LVM but Thin provisioning is "
                 "disabled.\n");
        return false;
    }
}

void LVM::checkEnoughDiskSpaceAvailable()
{
    std::list<std::string> output;
    const std::string checkDiskSpaceCommand
        = "vgs --noheadings -o vg_name,vg_size,vg_free --units G";

    int exitCode = cloyster::runCommand(checkDiskSpaceCommand, output);

    if (exitCode == 0 && !output.empty()) {
        try {
            bool allVGsHaveEnoughSpace = true;

            for (const std::string& line : output) {
                LOG_DEBUG("LVM: Check disks space - Raw output line: {}", line);

                // Remove leading/trailing whitespaces
                std::string cleanLine = line;
                cleanLine.erase(cleanLine.begin(),
                    std::find_if(cleanLine.begin(), cleanLine.end(),
                        [](unsigned char ch) { return !std::isspace(ch); }));
                cleanLine.erase(
                    std::find_if(cleanLine.rbegin(), cleanLine.rend(),
                        [](unsigned char ch) { return !std::isspace(ch); })
                        .base(),
                    cleanLine.end());

                if (cleanLine.empty()) {
                    LOG_DEBUG("LVM: Skipping empty or malformed line.");
                    continue;
                }

                // Split the line into vg_name, vg_size, and vg_free
                std::istringstream iss(cleanLine);
                std::string vgName, vgSizeStr, vgFreeStr;
                if (!(iss >> vgName >> vgSizeStr >> vgFreeStr)) {
                    LOG_DEBUG("LVM: Failed to split line into expected fields. "
                              "Line: {}",
                        cleanLine);
                    throw std::runtime_error(
                        "LVM ERROR: Failed to parse volume group information.");
                }

                // Remove 'G' from size and free space strings
                vgSizeStr.erase(
                    std::remove(vgSizeStr.begin(), vgSizeStr.end(), 'G'),
                    vgSizeStr.end());
                vgFreeStr.erase(
                    std::remove(vgFreeStr.begin(), vgFreeStr.end(), 'G'),
                    vgFreeStr.end());

                // Convert vg_size and vg_free from string to double
                double vgSizeGB = std::stod(vgSizeStr);
                double vgFreeGB = std::stod(vgFreeStr);

                // Calculate the percentage of free space
                double freePercentage = (vgFreeGB / vgSizeGB) * 100;

                LOG_TRACE("LVM Volume Group: {}, Total Size: {} GB, Free "
                          "Space: {} GB, Free Percentage: {:.2f}%",
                    vgName, vgSizeGB, vgFreeGB, freePercentage);

                // Check if the free space is less than 50%
                if (freePercentage < 50.0) {
                    LOG_WARN("LVM Volume Group '{}' does not have enough free "
                             "space (only {:.2f}% available).",
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
    int exitCode = cloyster::runCommand(checkPartitionsCommand, output);

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
            LOG_TRACE(
                "LVM: /home is in a separate partition. We can proceed with "
                "LVM snapshot.");
        } else {
            LOG_WARN("LVM: /home is not a separate partition. This may lead "
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

    if (isLVMEnabled()) {
        isRootLVMEnabled();
        verifyBootIsNotLVM();
        isThinProvisioningEnabled();
        isRootThinProvisioningEnabled();
        checkEnoughDiskSpaceAvailable();
        verifyAvailablePartitions();
    }

    LOG_INFO("LVM: End of availability check.")
}

bool LVM::snapshotExists(const std::string& snapshotName)
{
    std::list<std::string> output;

    const std::string checkSnapshotCommand
        = fmt::format("lvs --noheadings -o lv_name {}/{}",
            m_snapshotVolumeGroup, snapshotName);
    int exitCode = cloyster::runCommand(checkSnapshotCommand, output);

    // If the command succeeded (exitCode 0) and output is not empty, snapshot
    // exists
    if (exitCode == 0 && !output.empty()) {
        std::string lvName = output.front();

        // Trim leading/trailing whitespaces
        lvName.erase(lvName.begin(),
            std::find_if(lvName.begin(), lvName.end(),
                [](unsigned char ch) { return !std::isspace(ch); }));
        lvName.erase(std::find_if(lvName.rbegin(), lvName.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                         .base(),
            lvName.end());

        // Check if the name matches the expected snapshot name
        if (lvName == snapshotName) {
            LOG_INFO("LVM: Snapshot {} exists.", snapshotName);
            return true;
        } else {
            LOG_INFO("LVM: Snapshot found but name mismatch: expected '{}', "
                     "got '{}'",
                snapshotName, lvName);
        }
    }

    // If the command failed or the snapshot was not found
    LOG_INFO("LVM: Snapshot {} does not exist.", snapshotName);
    return false;
}

void LVM::verifyBootIsNotLVM()
{
    std::list<std::string> output;

    const std::string listCommand
        = "lsblk --noheadings --output MOUNTPOINT,TYPE";
    int exitCodeList = cloyster::runCommand(listCommand, output);

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
        throw std::runtime_error("LVM ERROR: /boot is part of LVM. Check your "
                                 "system configuration.");
    } else {
        LOG_WARN("LVM: /boot is not found or is part of LVM. Check your system "
                 "configuration.");
    }
}

void LVM::backupBoot()
{
    //@TODO We need a better way to store the backup path
    cloyster::createDirectory(fmt::format("/opt/{}/backup", PRODUCT_NAME));

    //@TODO We need a better way to store the backup path
    const std::string backupCommand
        = fmt::format("rsync -a /boot/ /opt/{}/backup/boot/", PRODUCT_NAME);

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
        = fmt::format("rsync -a /opt/{}/backup/boot/ /boot/", PRODUCT_NAME);

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

    int exitCode = cloyster::runCommand(checkVGCommand, output);

    if (exitCode == 0 && !output.empty()) {
        // Assuming we want to get the first volume group found
        std::string vgName = output.front();

        // Trim any leading/trailing whitespaces
        vgName.erase(vgName.begin(),
            std::find_if(vgName.begin(), vgName.end(),
                [](unsigned char ch) { return !std::isspace(ch); }));
        vgName.erase(std::find_if(vgName.rbegin(), vgName.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                         .base(),
            vgName.end());

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
    if (snapshotExists(snapshotName)) {
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
    if (!snapshotExists(snapshotName)) {
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
    if (!snapshotExists(snapshotName)) {
        return;
    }

    LOG_INFO("LVM Snapshot removal in progress.");
    checkVolumeGroup();

    std::list<std::string> output;
    const std::string checkSnapshotCommand
        = fmt::format("lvs --noheadings -o lv_attr {}/{}",
            m_snapshotVolumeGroup, snapshotName);

    int exitCode = cloyster::runCommand(checkSnapshotCommand, output);

    if (exitCode == 0 && !output.empty()) {
        std::string lvAttr = output.front();

        // Check if the snapshot is merged (state will be inactive or missing)
        if (lvAttr.find("M") != std::string::npos) {
            LOG_WARN(
                "LVM Snapshot '{}' is already merged and cannot be removed.",
                snapshotName);
        } else {
            const std::string removeSnapshotCommand = fmt::format(
                "lvremove {}/{}", m_snapshotVolumeGroup, snapshotName);
            exitCode = cloyster::runCommand(removeSnapshotCommand);

            if (exitCode == 0) {
                LOG_INFO(
                    "LVM Snapshot '{}' removed successfully.", snapshotName);
            } else {
                throw std::runtime_error(
                    fmt::format("LVM ERROR: Failed to remove snapshot '{}'.",
                        snapshotName));
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

LVM::LVM() { checkVolumeGroup(); }
