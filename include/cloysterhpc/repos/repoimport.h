/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOIMPORT_H_
#define CLOYSTERHPC_REPOIMPORT_H_

#include <fmt/format.h>
#include <string>
#include <vector>

enum class AdditionalType {
    beegfs,
    ELRepo,
    EPEL,
    Grafana,
    influxData,
    oneAPI,
    OpenHPC,
    Zabbix,
    RPMFusionUpdates
};

class Distro {
public:
    std::string repo_gpg;
    std::string repo_gpg_filename;
    std::vector<std::string> dependencies;

    Distro(const std::string& repo_gpg, const std::string& repo_gpg_filename,
        const std::vector<std::string> dependencies)
        : repo_gpg(repo_gpg)
        , repo_gpg_filename(repo_gpg_filename)
        , dependencies(dependencies)
    {
    }

    Distro() = default;

    [[nodiscard]] std::string joinDependencies() const
    {
        return fmt::to_string(fmt::join(dependencies, ","));
    }
};

class AdditionalRepo {
public:
    AdditionalType type;
    std::string gpg_filename;
    std::string gpg_key;

    AdditionalRepo(const AdditionalType type, const std::string& gpg_filename,
        const std::string& gpg_key)
        : type(type)
        , gpg_filename(gpg_filename)
        , gpg_key(gpg_key)
    {
    }

    AdditionalRepo() = default;
};

class Family {
public:
    std::string repo;
    std::vector<AdditionalRepo> additionalRepos;
    Distro RHEL;
    Distro AlmaLinux;
    Distro Rocky;
    Distro Oracle;

    Family() = default;

    Family(const std::string& repo,
        const std::vector<AdditionalRepo> additionalRepos, const Distro& rhel,
        const Distro& almalinux, const Distro& rocky, const Distro& oracle)
        : repo(repo)
        , additionalRepos(additionalRepos)
        , RHEL(rhel)
        , AlmaLinux(almalinux)
        , Rocky(rocky)
        , Oracle(oracle)
    {
    }
};

const Family EL8(
#include "cloysterhpc/repos/el8/cloyster.repo"
    , { AdditionalRepo({
            AdditionalType::beegfs,
            "RPM-GPG-KEY-beegfs",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-beegfs"
        }),
          AdditionalRepo({
              AdditionalType::ELRepo,
              "RPM-GPG-KEY-elrepo",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-elrepo"
          }),
          AdditionalRepo({
              AdditionalType::EPEL,
              "RPM-GPG-KEY-epel",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-epel"
          }),
          AdditionalRepo({
              AdditionalType::Grafana,
              "RPM-GPG-KEY-grafana",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-grafana"
          }),
          AdditionalRepo({
              AdditionalType::influxData,
              "RPM-GPG-KEY-influxdata",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-influxdata"
          }),
          AdditionalRepo({
              AdditionalType::oneAPI,
              "RPM-GPG-KEY-oneapi",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-oneapi"
          }),
          AdditionalRepo({
              AdditionalType::OpenHPC,
              "RPM-GPG-KEY-openhpc",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-openhpc"
          }),
          AdditionalRepo({
              AdditionalType::RPMFusionUpdates,
              "RPM-GPG-KEY-rpmfusion-updates",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-rpmfusion-updates"
          }),
          AdditionalRepo({
              AdditionalType::Zabbix,
              "RPM-GPG-KEY-zabbix",
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-zabbix"
          }) },
    Distro({
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-rhel-unavailable"
        ,
        "RPM-GPG-KEY-rhel-unavailable",
        { "codeready-builder-for-rhel-8-x86_64-rpms" },
    }),
    Distro({
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-AlmaLinux"
        ,
        "RPM-GPG-KEY-AlmaLinux",
        { "cloyster-AlmaLinux-BaseOS", "powertools" },
    }),
    Distro({
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-rockyofficial"
        ,
        "RPM-GPG-KEY-rockyofficial",
        { "cloyster-Rocky-BaseOS", "powertools" },
    }),
    Distro({
#include "cloysterhpc/repos/el8/rpm-gpg/RPM-GPG-KEY-oracle"
        , "RPM-GPG-KEY-oracle",
        { "cloyster-OL-BaseOS", "ol8_codeready_builder" } }));

const Family EL9(
#include "cloysterhpc/repos/el9/cloyster.repo"
    , { AdditionalRepo({
            AdditionalType::beegfs,
            "RPM-GPG-KEY-beegfs",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-beegfs"
        }),
          AdditionalRepo({
              AdditionalType::ELRepo,
              "RPM-GPG-KEY-elrepo",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-elrepo"
          }),
          AdditionalRepo({
              AdditionalType::EPEL,
              "RPM-GPG-KEY-epel",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-epel"
          }),
          AdditionalRepo({
              AdditionalType::Grafana,
              "RPM-GPG-KEY-grafana",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-grafana"
          }),
          AdditionalRepo({
              AdditionalType::influxData,
              "RPM-GPG-KEY-influxdata",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-influxdata"
          }),
          AdditionalRepo({
              AdditionalType::oneAPI,
              "RPM-GPG-KEY-oneapi",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-oneapi"
          }),
          AdditionalRepo({
              AdditionalType::OpenHPC,
              "RPM-GPG-KEY-openhpc",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-openhpc"
          }),
          AdditionalRepo({
              AdditionalType::RPMFusionUpdates,
              "RPM-GPG-KEY-rpmfusion-updates",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-rpmfusion-updates"
          }),
          AdditionalRepo({
              AdditionalType::Zabbix,
              "RPM-GPG-KEY-zabbix",
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-zabbix"
          }) },
    Distro({
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-rhel-unavailable"
        ,
        "RPM-GPG-KEY-rhel-unavailable",
        { "codeready-builder-for-rhel-9-x86_64-rpms" },
    }),
    Distro({
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-AlmaLinux"
        ,
        "RPM-GPG-KEY-AlmaLinux",
        { "cloyster-AlmaLinux-BaseOS", "crb" },
    }),
    Distro({
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-rockyofficial"
        ,
        "RPM-GPG-KEY-rockyofficial",
        { "cloyster-Rocky-BaseOS", "crb" },
    }),
    Distro({
#include "cloysterhpc/repos/el9/rpm-gpg/RPM-GPG-KEY-oracle"
        , "RPM-GPG-KEY-oracle",
        { "cloyster-OL-BaseOS", "ol9_codeready_builder" } }));

#endif // CLOYSTERHPC_REPOIMPORT_H_
