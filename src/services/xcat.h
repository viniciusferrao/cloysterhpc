#ifndef XCAT_H
#define XCAT_H

#include "provisioner.h"
#include "execution.h"
#include "shell.h"

#include <string>
#include <memory>

class XCAT : public Provisioner {
private:
    Shell& m_executionEngine;

public:
    XCAT(Shell&);
    ~XCAT();

    void setDHCPInterfaces (std::string);
    void setDomain (std::string);
    void copycds (std::string);
    void genimage (std::string);
    void addOpenHPCComponents (std::string);
};

#endif /* XCAT_H */