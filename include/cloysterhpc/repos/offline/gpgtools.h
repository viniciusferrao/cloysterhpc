/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_GPGTOOLS_H_
#define CLOYSTERHPC_GPGTOOLS_H_

#include "gpgkeys.h"
#include <algorithm>
#include <fmt/format.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

enum class AddonType {
    beegfs,
    ELRepo,
    EPEL,
    Grafana,
    influxData,
    oneAPI,
    OpenHPC,
    Zabbix,
    RPMFusionUpdates,
    NvidiaHPCSDK
};

enum class Platform { el8, el9 };

class GPGVerifier {
protected:
    std::multimap<AddonType, std::pair<Platform, std::string>> m_gpg_keys;

public:
    GPGVerifier()
        : m_gpg_keys { { AddonType::beegfs,
                           { Platform::el8, GPG_KEY_BEEGFS_8 } },
            { AddonType::beegfs, { Platform::el9, GPG_KEY_BEEGFS_9 } },
            { AddonType::ELRepo, { Platform::el8, GPG_KEY_ELREPO_8 } },
            { AddonType::ELRepo, { Platform::el9, GPG_KEY_ELREPO_9 } },
            { AddonType::EPEL, { Platform::el8, GPG_KEY_EPEL_8 } },
            { AddonType::EPEL, { Platform::el9, GPG_KEY_EPEL_9 } },
            { AddonType::Grafana, { Platform::el8, GPG_KEY_GRAFANA_8 } },
            { AddonType::Grafana, { Platform::el9, GPG_KEY_GRAFANA_9 } },
            { AddonType::influxData, { Platform::el8, GPG_KEY_INFLUXDATA_8 } },
            { AddonType::influxData, { Platform::el9, GPG_KEY_INFLUXDATA_9 } },
            { AddonType::oneAPI, { Platform::el8, GPG_KEY_ONEAPI_8 } },
            { AddonType::oneAPI, { Platform::el9, GPG_KEY_ONEAPI_9 } },
            { AddonType::OpenHPC, { Platform::el8, GPG_KEY_OPENHPC_8 } },
            { AddonType::OpenHPC, { Platform::el9, GPG_KEY_OPENHPC_9 } },
            { AddonType::RPMFusionUpdates,
                { Platform::el8, GPG_KEY_RPMFUSIONUPDATES_8 } },
            { AddonType::RPMFusionUpdates,
                { Platform::el9, GPG_KEY_RPMFUSIONUPDATES_9 } },
            { AddonType::Zabbix, { Platform::el8, GPG_KEY_ZABBIX_8 } },
            { AddonType::Zabbix, { Platform::el9, GPG_KEY_ZABBIX_9 } },
            { AddonType::NvidiaHPCSDK, { Platform::el8, GPG_KEY_NVHPCSDK_8 } },
            { AddonType::NvidiaHPCSDK, { Platform::el9, GPG_KEY_NVHPCSDK_9 } } }
    {
    }

    std::vector<std::pair<Platform, std::string>> getGPGKeys(AddonType type)
    {
        std::vector<std::pair<Platform, std::string>> result;

        auto range = m_gpg_keys.equal_range(type);
        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(it->second);
        }

        return result;
    }

    bool verifyGPGKey(
        AddonType type, Platform platform, const std::string& gpgKey)
    {
        auto range = m_gpg_keys.equal_range(type);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.first == platform && it->second.second == gpgKey) {
                return true;
            }
        }
        return false;
    }
};

#endif // CLOYSTERHPC_GPGTOOLS_H_