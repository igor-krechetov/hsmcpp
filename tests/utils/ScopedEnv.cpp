#include "utils/ScopedEnv.hpp"

#include <cstdlib>

ScopedEnv::ScopedEnv(const char* name, const std::string& value)
    : mName(name) {
    const char* prev = std::getenv(name);

    mHad = (nullptr != prev);

    if (mHad) {
        mPrev = prev;
    }

    set(value);
}

ScopedEnv::ScopedEnv(const char* name)
    : mName(name) {
    const char* prev = std::getenv(name);

    mHad = (nullptr != prev);
    
    if (mHad) {
        mPrev = prev;
    }
    unset();
}

ScopedEnv::~ScopedEnv() {
    if (mHad) {
        set(mPrev);
    } else {
        unset();
    }
}

void ScopedEnv::set(const std::string& v) {
#ifdef _WIN32
    _putenv_s(mName.c_str(), v.c_str());
#else
    ::setenv(mName.c_str(), v.c_str(), 1);
#endif
}

void ScopedEnv::unset() {
#ifdef _WIN32
    _putenv_s(mName.c_str(), "");
#else
    ::unsetenv(mName.c_str());
#endif
}
