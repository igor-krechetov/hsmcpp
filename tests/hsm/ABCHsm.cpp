#include "ABCHsm.hpp"

ABCHsm::ABCHsm() : HierarchicalStateMachine(AbcState::A)
{
}

ABCHsm::~ABCHsm()
{
}

bool ABCHsm::conditionTrue(const VariantList_t& args)
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