//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 6/26/23.
//

#ifndef CLOYSTERHPC_ISERVICE_H_
#define CLOYSTERHPC_ISERVICE_H_

class IService {
public:
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif // CLOYSTERHPC_ISERVICE_H_
