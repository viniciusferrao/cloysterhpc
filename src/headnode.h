#ifndef HEADNODE_H
#define HEADNODE_H

#include <string>
#include <list>
#include <memory>

#include "server.h"
#include "network.h"
#include "connection.h"
#include "os.h"

class Headnode : public Server {
public:
    enum class BootTarget { Text, Graphical };

private:
    BootTarget m_bootTarget;

private:
    void discoverNames();

public:
    Headnode();

    [[nodiscard]] BootTarget getBootTarget() const;
    void setBootTarget(BootTarget bootTarget);
};

#endif /* HEADNODE_H */
