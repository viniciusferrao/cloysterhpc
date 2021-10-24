#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>

#include "cluster.h"
#include "headnode.h"
#include "terminalui.h"

class Controller {
private:
    Cluster* m_cluster;
    Headnode* m_headnode;
    TerminalUI* m_terminalui;

    void startView ();
    std::string requestTimezone ();
    std::string requestLocale ();
    std::vector<std::string> requestHostname ();
    std::string requestNetworkInterface ();
    std::vector<std::string> requestNetworkAddress ();

public:
    Controller (Cluster&);
    //~Controller ();
};

#endif /* CONTROLLER_H */

