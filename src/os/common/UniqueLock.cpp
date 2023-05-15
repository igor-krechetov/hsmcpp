// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/UniqueLock.hpp"

#include "hsmcpp/os/Mutex.hpp"

namespace hsmcpp {

UniqueLock::UniqueLock(Mutex& sync)
// NOTE: false-positive. thinks that ':' is arithmetic operation
// cppcheck-suppress misra-c2012-10.4
    : mSync(&sync) {
    lock();
}

UniqueLock::~UniqueLock() {
    unlock();
}

UniqueLock::UniqueLock(UniqueLock&& src) noexcept
    : mSync(src.mSync)
    , mOwnsLock(src.mOwnsLock) {
    src.mSync = nullptr;
    src.mOwnsLock = false;
}

UniqueLock& UniqueLock::operator=(UniqueLock&& src) noexcept {
    if (mOwnsLock) {
        unlock();
    }

    mSync = src.mSync;
    mOwnsLock = src.mOwnsLock;

    src.mSync = nullptr;
    src.mOwnsLock = false;

    return *this;
}

void UniqueLock::lock() {
    if ((nullptr != mSync) && (false == mOwnsLock)) {
        mSync->lock();
        mOwnsLock = true;
    }
}

void UniqueLock::unlock() {
    if ((nullptr != mSync) && (true == mOwnsLock)) {
        mSync->unlock();
        mOwnsLock = false;
    }
}

Mutex* UniqueLock::release() noexcept {
    Mutex* res = mSync;

    mSync = nullptr;
    mOwnsLock = false;

    return res;
}

}  // namespace hsmcpp
