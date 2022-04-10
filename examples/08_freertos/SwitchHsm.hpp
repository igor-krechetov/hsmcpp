// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#ifndef __EXAMPLES_08_FREERTOS_SWITCHHSM__
#define __EXAMPLES_08_FREERTOS_SWITCHHSM__

#include <stdio.h>
#include "gen/SwitchHsmBase.hpp"

class SwitchHsm: public SwitchHsmBase
{
public:
    virtual ~SwitchHsm(){}

// HSM state changed callbacks
protected:
    void onOff(const hsmcpp::VariantVector_t& args) override
    {
        printf("Off\n");
    }

    void onOn(const hsmcpp::VariantVector_t& args) override
    {
        printf("On\n");
    }

    void onPaused(const hsmcpp::VariantVector_t& args) override
    {
        printf("Paused\n");
    }
};

#endif // __EXAMPLES_08_FREERTOS_SWITCHHSM__