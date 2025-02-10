/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterGeneralSettings.h>
#include <cloysterhpc/presenter/PresenterHostId.h>
#include <cloysterhpc/presenter/PresenterInfiniband.h>
#include <cloysterhpc/presenter/PresenterInstall.h>
#include <cloysterhpc/presenter/PresenterInstructions.h>
#include <cloysterhpc/presenter/PresenterLocale.h>
#include <cloysterhpc/presenter/PresenterMailSystem.h>
#include <cloysterhpc/presenter/PresenterNetwork.h>
#include <cloysterhpc/presenter/PresenterNodes.h>
#include <cloysterhpc/presenter/PresenterNodesOperationalSystem.h>
#include <cloysterhpc/presenter/PresenterQueueSystem.h>
#include <cloysterhpc/presenter/PresenterRepository.h>
#include <cloysterhpc/presenter/PresenterTime.h>
#include <cloysterhpc/presenter/PresenterWelcome.h>

PresenterInstall::PresenterInstall(
    std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

#if 1 // Welcome messages
    Call<PresenterWelcome>();
    Call<PresenterInstructions>();
#endif

#if 1 // Set general settings
    Call<PresenterGeneralSettings>();
#endif

#if 1 // Timezone and locale support
    Call<PresenterTime>();
    Call<PresenterLocale>();
#endif

#if 1 // Hostname and domain
    Call<PresenterHostId>();
#endif

#if 1 // Repository
    Call<PresenterRepository>();
#endif

    NetworkCreator nc;
#if 1 // Networking
    // TODO: Under development
    //  * Add it to a loop where it asks to the user which kind of network we
    //  should add, while the operator says it's done adding networks. We remove
    //  the lazy network{1,2} after that.

    try {
        Call<PresenterNetwork>(nc, Network::Profile::External);
    } catch (const std::exception& ex) {
        LOG_ERROR("Failed to add {} network: {}",
            magic_enum::enum_name(Network::Profile::External), ex.what());
    }

    try {
        Call<PresenterNetwork>(nc, Network::Profile::Management);
    } catch (const std::exception& ex) {
        LOG_ERROR("Failed to add {} network: {}",
            magic_enum::enum_name(Network::Profile::Management), ex.what());
    }

#endif

#if 1 // Infiniband support
    Call<PresenterInfiniband>(nc);
#endif
    nc.saveNetworksToModel(*m_model);

#if 1 // Compute nodes formation details
    Call<PresenterNodesOperationalSystem>();
    Call<PresenterNodes>();
#endif

#if 1 // Queue System
    Call<PresenterQueueSystem>();
#endif

#if 1 // Mail system
    Call<PresenterMailSystem>();
#endif

    // Destroy the view since we don't need it anymore
    m_view.reset();
}
