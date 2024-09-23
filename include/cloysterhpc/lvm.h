/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LVM_H_
#define LVM_H_
#include <string>

class LVM {
private:
    bool m_hasVarPartition = false;
    bool m_hasOptPartition = false;
    bool m_hasHomePartition = false;
    bool m_hasBootPartition = false;
    std::string m_snapshotVolumeGroup;
    void verifyBootIsNotLVM();
    void backupBoot();
    void restoreBoot();
    void checkVolumeGroup();
    bool isUEFIModeEnabled();
    bool isLVMEnabled();
    bool isRootLVMEnabled();
    bool isThinProvisioningEnabled();
    bool isRootThinProvisioningEnabled();
    bool checkEnoughDiskSpaceAvailable();
    void verifyAvailablePartitions();

public:
    void checkLVMAvailability();
    bool snapshotExists(const std::string& snapshotName);
    void createSnapshot(const std::string& snapshotName);
    void rollbackSnapshot(const std::string& snapshotName);
    void removeSnapshot(const std::string& snapshotName);
    void createSnapshotWithBootBackup(const std::string& snapshotName);
    void rollbackSnapshotWithBootRestore(const std::string& snapshotName);
    LVM();
};

#endif // LVM_H_
