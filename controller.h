#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>

#include "cluster.h"
#include "headnode.h"
#include "terminalui.h"

class Controller {
    Cluster* cluster;
    Headnode* headnode;
    TerminalUI* terminalui;

    void startView ();
    std::string requestTimezone ();
    std::string requestLocale ();


public:
    Controller (Cluster&, Headnode&, TerminalUI&);
    //~Controller ();
};

#endif /* CONTROLLER_H */

