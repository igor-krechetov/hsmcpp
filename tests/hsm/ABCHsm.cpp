#include "ABCHsm.hpp"
#include "HsmEventDispatcherGLib.hpp"

ABCHsm::ABCHsm() : HierarchicalStateMachine(AbcState::A, std::make_shared<HsmEventDispatcherGLib>())
{
}

ABCHsm::~ABCHsm()
{}

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
}