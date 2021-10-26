//
// Created by Vinícius Ferrão on 24/10/21.
//

#include "presenterLocaleSelection.h"

PresenterLocaleSelection::PresenterLocaleSelection (View& view) {
    m_view = &view;
}

void PresenterLocaleSelection::write (Cluster& cluster) {
    this->m_locales = { "en.US_UTF-8", "pt.BR_UTF-8", "C" };

    cluster.locale = m_view->localeSelection(m_locales);
}
