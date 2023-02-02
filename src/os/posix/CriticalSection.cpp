// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/posix/CriticalSection.hpp"

namespace hsmcpp {
CriticalSection::CriticalSection() {
    sigset_t blockMask;

    // add all signals to the set
    sigfillset(&blockMask);
    // block signals and store original signals mask
    pthread_sigmask(SIG_BLOCK, &blockMask, &mOriginalSigMask);
}

CriticalSection::~CriticalSection() {
    // restore signals mask
    pthread_sigmask(SIG_SETMASK, &mOriginalSigMask, nullptr);
}

}  // namespace hsmcpp
