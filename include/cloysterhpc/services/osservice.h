#ifndef CLOYSTER_OSSERVICE_H_
#define CLOYSTER_OSSERVICE_H_

#include <string>
#include <vector>

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
 * recipes used by the installer. Also this interface is for getting runtime
 * information from the operating system, for in-memory state use
 * cloyster::models::OS class instead.
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

    // These methods are const because the implementation is expected to be stateless
    // The implementation is expect to be stateless to avoid double source of true,
    // the internal state vs the OS runtime state
    virtual bool install(std::string_view package) const = 0;
    virtual bool remove(std::string_view package) const = 0;
    virtual bool update(std::string_view package) const = 0;
    virtual bool update() const = 0;
    virtual void check() const = 0;
    virtual void clean() const = 0;
    virtual std::vector<std::string> repolist() const = 0;

    static std::unique_ptr<IOSService> factory(const OS& osinfo);
};

};

#endif
