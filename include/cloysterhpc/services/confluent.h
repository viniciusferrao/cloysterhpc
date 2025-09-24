#ifndef CLOYSTERHPC_CONFLUENT_H_
#define CLOYSTERHPC_CONFLUENT_H_

#include <cloysterhpc/services/provisioner.h>

namespace cloyster::services {

class Confluent final : public Provisioner<Confluent> {
public:
    static void install();
};

}

#endif
