#ifndef XCAT_H
#define XCAT_H

#include <string>

class XCAT {
public:
    void setDHCPInterfaces (std::string);
    void setDomain (std::string);
    void copycds (std::string);
    void genimage (std::string);
    void addOpenHPCComponents (std::string);
};

#endif /* XCAT_H */