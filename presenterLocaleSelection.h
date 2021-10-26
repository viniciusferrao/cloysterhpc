//
// Created by Vinícius Ferrão on 24/10/21.
//

#ifndef PRESENTERLOCALESELECTION_H
#define PRESENTERLOCALESELECTION_H

#include "view.h"
#include "cluster.h"

class PresenterLocaleSelection {
    View *m_view;
    std::vector<std::string> m_locales;

public:
    explicit PresenterLocaleSelection (View&);
    void write (Cluster&);
};

#endif /* PRESENTERLOCALESELECTION_H */
