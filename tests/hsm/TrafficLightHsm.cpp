#include "TrafficLightHsm.hpp"

TrafficLightHsm::TrafficLightHsm() : HierarchicalStateMachine(TrafficLightState::OFF)
{
}

TrafficLightHsm::~TrafficLightHsm()
{}

void TrafficLightHsm::setupDefault()
{
    registerState(TrafficLightState::OFF, this, &TrafficLightHsm::onOff, nullptr, nullptr);
    registerState(TrafficLightState::STARTING, this, &TrafficLightHsm::onStarting, nullptr, nullptr);
    registerState(TrafficLightState::RED, this, &TrafficLightHsm::onRed, nullptr, nullptr);
    registerState(TrafficLightState::YELLOW, this, &TrafficLightHsm::onYellow, nullptr, nullptr);
    registerState(TrafficLightState::GREEN, this, &TrafficLightHsm::onGreen, nullptr, nullptr);

    ASSERT_TRUE(registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::RED, true));
    ASSERT_TRUE(registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::YELLOW));
    ASSERT_TRUE(registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::GREEN));

    registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);
    registerTransition(TrafficLightState::OPERABLE, TrafficLightState::OFF, TrafficLightEvent::TURN_OFF, this, &TrafficLightHsm::onTurnOffTransition);
    registerTransition(TrafficLightState::STARTING, TrafficLightState::OPERABLE, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
    registerTransition(TrafficLightState::RED, TrafficLightState::YELLOW, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
    registerTransition(TrafficLightState::YELLOW, TrafficLightState::GREEN, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
    registerTransition(TrafficLightState::GREEN, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE, this, &TrafficLightHsm::onNextStateTransition);
}

void TrafficLightHsm::SetUp()
{
}

void TrafficLightHsm::TearDown()
{
}