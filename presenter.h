#ifndef PRESENTER_H
#define PRESENTER_H

#include <string>

#include "cluster.h"
#include "headnode.h"
#include "terminalui.h"

class Presenter {
private:
    Cluster* m_cluster{};
    TerminalUI* m_terminalui{};

    void startView ();
    std::string requestTimezone ();
    std::string requestLocale ();
    std::vector<std::string> requestHostname ();
    std::string requestNetworkInterface ();
    std::vector<std::string> requestNetworkAddress ();

public:
    explicit Presenter (Cluster&);
    //~Presenter ();
};

#endif /* PRESENTER_H */

