// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/common/CriticalSection.hpp"

#include "hsmcpp/os/InterruptsFreeSection.hpp"
#include "hsmcpp/os/Mutex.hpp"

namespace hsmcpp {

CriticalSection::CriticalSection(Mutex& sync)
    : mSync(sync)
    , mInterruptsBlock(new InterruptsFreeSection()) {
    mSync.lock();
}

CriticalSection::~CriticalSection() {
    mSync.unlock();
}

}  // namespace hsmcpp
