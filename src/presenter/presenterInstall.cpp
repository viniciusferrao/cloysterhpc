//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenterInstall.h"
#include "presenterWelcome.h"
#include "PresenterInstructions.h"
#include "PresenterGeneralSettings.h"
#include "PresenterTime.h"
#include "PresenterLocale.h"
#include "PresenterHostId.h"
#include "presenterNetwork.h"
#include "PresenterInfiniband.h"
#include "PresenterNodes.h"
#include "PresenterQueueSystem.h"
#include "PresenterMailSystem.h"

PresenterInstall::PresenterInstall(std::unique_ptr<Cluster>& model,
                                   std::unique_ptr<Newt>& view)
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

#if 1 // Networking
    // TODO: Under development
    //  * Add it to a loop where it asks to the user which kind of network we
    //  should add, while the operator says it's done adding networks. We remove
    //  the lazy network{1,2} after that.

    try {
        Call<PresenterNetwork>();
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::External), ex.what());
    }

    try {
        Call<PresenterNetwork>(Network::Profile::Management);
        //PresenterNetwork network(model, view, Network::Profile::Management);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::Management), ex.what());
    }
#endif

#if 1 // Infiniband support
    Call<PresenterInfiniband>();
#endif

#if 1 // Compute nodes formation details
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
