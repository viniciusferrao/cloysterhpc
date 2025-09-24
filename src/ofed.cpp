/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fmt/core.h>

#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/ofed.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/utils/singleton.h>
#include <utility>

using cloyster::functions::IRunner;

void OFED::setKind(Kind kind) { m_kind = kind; }

OFED::Kind OFED::getKind() const { return m_kind; }

bool OFED::installed() const
{
    const auto opts = cloyster::utils::singleton::options();
    if (opts->shouldForce("infiniband-install")) {
        return false;
    }

    // Return false so the installation runs on dry run
    if (opts->dryRun) {
        return false;
    }

    auto runner = cloyster::Singleton<IRunner>::get();
    switch (m_kind) {
        case OFED::Kind::Mellanox:
            return runner->executeCommand("rpm -q doca-ofed") == 0;
        case OFED::Kind::Inbox:
            return runner->executeCommand(
                       "dnf group info \"Infiniband Support\"")
                == 0;
        case OFED::Kind::Oracle:
            throw std::logic_error("Not implemented");
    }

    std::unreachable();
}

void OFED::install() const
{
    const auto opts = cloyster::utils::singleton::options();

    if (opts->dryRun) {
        LOG_WARN("Dry-Run: Skiping OFED installation");
        return;
    }

    // Idempotency check
    if (installed()) {
        LOG_WARN("Inifiniband already installed, skipping, use `--force "
                 "infiniband-install` to force");
        return;
    }

    switch (m_kind) {
        case OFED::Kind::Inbox:
            cloyster::Singleton<cloyster::services::IOSService>::get()
                ->groupInstall("Infiniband Support");
            break;

        case OFED::Kind::Mellanox: {
            auto runner
                = cloyster::Singleton<cloyster::services::IRunner>::get();
            auto repoManager = cloyster::Singleton<
                cloyster::services::repos::RepoManager>::get();
            const auto kernelVersion
                = cloyster::utils::singleton::answerfile()->system.kernel;

            repoManager->enable("doca");
            // Install the required packages
            runner->checkCommand("dnf makecache --repo=doca");
            if (kernelVersion) {
                LOG_WARN("Building OFED with kernel version from the "
                         "answerfile {} @ [system].kernel: {}",
                    cloyster::utils::singleton::answerfile()->path(),
                    kernelVersion.value());
                runner->checkCommand(
                    fmt::format("dnf -y install --nogpg kernel-{kernelVersion} "
                                "kernel-devel-{kernelVersion} doca-extra",
                        fmt::arg("kernelVersion", kernelVersion.value())));
            } else {
                runner->checkCommand(
                    "dnf -y --nogpg install kernel kernel-devel doca-extra");
            }

            LOG_INFO("Compiling OFED DOCA drivers, this may take a while, use "
                     "`--skip compile-doca-driver` to skip");
            if (kernelVersion) {
                if (!opts->shouldSkip("compile-doca-driver")) {
                    runner->checkCommand(fmt::format(
                        "/opt/mellanox/doca/tools/doca-kernel-support -k {}",
                        kernelVersion.value()));
                }
            } else {
                runner->checkCommand(fmt::format(
                    "/opt/mellanox/doca/tools/doca-kernel-support"));
            }

            // Get the last rpm in /tmp/DOCA*/ folder
            // On dry-run the below command will not run so we
            // cannot get the output of it
            auto rpm = runner->checkOutput(
                "bash -c \"find /tmp/DOCA*/ -name '*.rpm' -printf '%T@ %p\n' | "
                "sort -nk1 | tail -1 | awk '{print $2}'\"");
            assert(rpm.size() > 0); // at last one line

            // Install the (last) generated rpm
            runner->executeCommand(fmt::format("dnf install -y {}", rpm[0]));

            runner->checkCommand("dnf makecache --repo=doca*");
            runner->checkCommand(
                "dnf install --nogpg -y doca-ofed mlnx-fw-updater");
            runner->executeCommand("systemctl restart openibd");
            // runner->executeCommand("systemctl enable --now opensmd");
        } break;

        case OFED::Kind::Oracle:
            throw std::logic_error("Oracle RDMA release is not yet supported");

            break;
    }
}
