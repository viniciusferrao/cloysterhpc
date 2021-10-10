#ifndef _HPP_XCAT_
#define _HPP_XCAT_

#include <string>

class XCAT {
public:
    void setDHCPInterfaces (std::string);
    void setDomain (std::string);
    void copycds (std::string);
    void genimage (std::string);
    void addOpenHPCComponents (std::string);
};

#endif /* _HPP_XCAT_ */