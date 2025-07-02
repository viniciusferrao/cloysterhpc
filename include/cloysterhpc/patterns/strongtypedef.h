#ifndef CLOYSTERHPC_PATTERNS_STRONGTYPEDEF_H_
#define CLOYSTERHPC_PATTERNS_STRONGTYPEDEF_H_

namespace cloyster {

// NewType template (from your previous context)
template <typename T, typename Tag>
struct Wrapper {
    T value;
    explicit Wrapper(const T& v) : value(v) {}
    explicit operator T() const { return value; }
};

};

#endif
