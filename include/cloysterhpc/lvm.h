/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LVM_H
#define LVM_H
#include <string>;

class LVM {
private:
    bool m_hasVarPartition = false;
    bool m_hasOptPartition = false;
    bool m_hasHomePartition = false;
    bool git status = false;
    const std::string m_snapshotVolumeGroup = "cloyster";
    void verifyBootIsNotLVM();
    void backupBoot();
    void restoreBoot();
    void checkUEFIMode();
    void checkLVMEnabled();
    void checkThinProvisioning();
    void checkEnoughDiskSpaceAvailable();
    void verifyAvailablePartitions();

public:
    void createSnapshot(const std::string& snapshotName);
    void rollbackSnapshot(const std::string& snapshotName);
    void removeSnapshot(const std::string& snapshotName);
    void createSnapshotWithBootBackup(const std::string& snapshotName);
    void rollbackSnapshotWithBootRestore(const std::string& snapshotName);
    LVM();
};

#endif // LVM_H
