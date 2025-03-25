#ifndef CLOYSTER_SINGLETON_H
#define CLOYSTER_SINGLETON_H

#include <memory>
#include <mutex>
#include <gsl/gsl-lite.hpp>

namespace cloyster {

/**
 * @brief This class implement the Singleton pattern.
 * @details The user initialize a class C with Singleton<C>::init(value),
 *   and then get the instance with Singleton<C>::get().
 */
template <typename T>
class Singleton final {
    // Private constructor to prevent direct instantiation
    Singleton() = default;

    // Static members for singleton management
    static std::unique_ptr<T> instance;
    static std::once_flag initFlag;
public:
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    ~Singleton() = delete;

    static void init(std::unique_ptr<T> value)
    {
        std::call_once(initFlag, [&](){
            instance = std::move(value);
        });
    }

    static void init(const auto& factory)
    {
        std::call_once(initFlag, [&](){
            instance = std::move(factory());
        });
    }

    static gsl::not_null<T*> get() {
        if (!instance) {
            throw std::runtime_error("Singleton read before initialization");
        }
        return gsl::not_null<T*>(instance.get());
    }
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initFlag;

}


#endif
