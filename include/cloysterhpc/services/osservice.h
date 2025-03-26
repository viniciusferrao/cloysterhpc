#ifndef CLOYSTER_OSSERVICE_H_
#define CLOYSTER_OSSERVICE_H_

#include <string>

#include <cloysterhpc/models/os.h>

namespace cloyster::services {

using cloyster::models::OS;

/**
 * @brief Abstract operating system utilities, like probing kernel version or
 * packages installed. Use this class to avoid spreading distro dependent commands
 * in the installation repices.
 *
 * @details The implementations are hidden in the object file, client code
 * should usually consume this behind a Singleton<IOSService> pointer to ensure
 * proper dynamic dispatch. The initialization uses factory function to generate
 * the proper instance at runtime. The singleton is intialized based the cluster's
 * headnode OS instance.
 *
 * This interface must be enough to absctract OS command from the installation
 * recipes used by the installer.
 * 
 * @see See main.cpp for intialization and osservice.cpp for more info.
 */
class IOSService {
public:
    [[nodiscard]] IOSService(const IOSService&) = default;
    IOSService(IOSService&&) = delete;
    IOSService& operator=(const IOSService&) = default;
    IOSService& operator=(IOSService&&) = delete;
    IOSService() = default;
    virtual ~IOSService() = default;

    [[nodiscard]] virtual std::string getKernelInstalled() const = 0;
    [[nodiscard]] virtual std::string getKernelRunning() const = 0;

    static std::unique_ptr<IOSService> factory(const OS& osinfo);
};

};

#endif
