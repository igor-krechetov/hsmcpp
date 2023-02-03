// Copyright (C) 2023 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "hsmcpp/os/posix/InterruptsFreeSection.hpp"

namespace hsmcpp {
    
InterruptsFreeSection::InterruptsFreeSection() {
    sigset_t blockMask;

    // add all signals to the set
    sigfillset(&blockMask);
    // block signals and store original signals mask
    pthread_sigmask(SIG_BLOCK, &blockMask, &mOriginalSigMask);
}

InterruptsFreeSection::~InterruptsFreeSection() {
    // restore signals mask
    pthread_sigmask(SIG_SETMASK, &mOriginalSigMask, nullptr);
}

}  // namespace hsmcpp
