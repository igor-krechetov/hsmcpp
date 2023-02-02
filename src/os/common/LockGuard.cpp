// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/LockGuard.hpp"

#include "hsmcpp/os/Mutex.hpp"

namespace hsmcpp {

LockGuard::LockGuard(Mutex& sync)
    : mSync(sync) {
    mSync.lock();
}

LockGuard::~LockGuard() {
    mSync.unlock();
}

}  // namespace hsmcpp
