#ifndef CLOYSTER_SINGLETON_H_
#define CLOYSTER_SINGLETON_H_

#include <gsl/gsl-lite.hpp>
#include <memory>
#include <mutex>

namespace cloyster {

/**
 * @brief This class implement the Singleton pattern.
 * @details The user initialize a class C with Singleton<C>::init(value),
 *   and then get the instance with Singleton<C>::get().
 */
template <typename T> class Singleton final {
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
#ifdef BUILD_TESTING 
        // Allow reinitialization during the tests
        instance = std::move(value);
#else
        std::call_once(initFlag, [&]() { instance = std::move(value); });
#endif
    }

    static void init(const auto& factory)
    {
#ifdef BUILD_TESTING 
        instance = std::move(factory());
#else
        std::call_once(initFlag, [&]() { instance = std::move(factory()); });
#endif
    }

    static gsl::not_null<T*> get()
    {
        if (!instance) {
            throw std::runtime_error("Singleton read before initialization");
        }
        return gsl::not_null<T*>(instance.get());
    }
};

template <typename T> std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T> std::once_flag Singleton<T>::initFlag;

}

#endif
