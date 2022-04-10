// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "ABCHsm.hpp"

ABCHsm::ABCHsm() : HierarchicalStateMachine(AbcState::A)
{
}

ABCHsm::~ABCHsm()
{
}

bool ABCHsm::conditionTrue(const VariantVector_t& args)
{
    ++mConditionTrueCounter;
    mArgsConditionTrue = args;
    return true;
}

void ABCHsm::SetUp()
{
}

void ABCHsm::TearDown()
{
    RELEASE_HSM();
}