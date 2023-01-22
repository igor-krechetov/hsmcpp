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

std::string ABCHsm::getStateName(const AbcState state) const
{
    std::string res;

    switch(state)
    {
        case AbcState::A:
            res = "A";
            break;
        case AbcState::B:
            res = "B";
            break;
        case AbcState::C:
            res = "C";
            break;    
        case AbcState::D:
            res = "D";
            break;    
        case AbcState::E:
            res = "E";
            break;    
        case AbcState::F:
            res = "F";
            break;    
        case AbcState::H:
            res = "H";
            break;    
        case AbcState::H2:
            res = "H2";
            break;    
        case AbcState::P1:
            res = "P1";
            break;    
        case AbcState::P2:
            res = "P2";
            break;    
        case AbcState::P3:
            res = "P3";
            break;    
        case AbcState::P4:
            res = "P4";
            break;    
        case AbcState::F1:
            res = "F1";
            break;    
        case AbcState::F2:
            res = "F2";
            break;    
        case AbcState::F3:
            res = "F3";
            break;
        default:
            res = std::to_string(static_cast<int>(state));
            break;
    }

    return res;
}

std::string ABCHsm::getEventName(const AbcEvent event) const
{
    std::string res;

    switch(event)
    {
        case AbcEvent::E1:
            res = "E1";
            break;
        case AbcEvent::E2:
            res = "E2";
            break;
        case AbcEvent::E3:
            res = "E3";
            break;
        case AbcEvent::EXIT1:
            res = "EXIT1";
            break;
        case AbcEvent::EXIT2:
            res = "EXIT2";
            break;
        default:
            res = std::to_string(static_cast<int>(event));
            break;
    }

    return res;
}

void ABCHsm::SetUp()
{
}

void ABCHsm::TearDown()
{
    RELEASE_HSM();
}