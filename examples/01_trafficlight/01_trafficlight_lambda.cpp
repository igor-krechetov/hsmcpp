#include <unistd.h>
#include "hsmcpp/hsm.hpp"
#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

enum class TrafficLightState
{
    OFF,

    OPERABLE,
    INITIALIZING,
    RED,
    YELLOW,
    GREEN
};

enum class TrafficLightEvent
{
    POWER_ON,
    POWER_OFF,
    INIT_DONE,
    NEXT_STATE
};

int main(const int argc, const char**argv)
{
    Glib::init();

    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    HierarchicalStateMachine<TrafficLightState, TrafficLightEvent> hsm(TrafficLightState::OFF);

    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());

    hsm.registerState(TrafficLightState::OFF, [mainLoop](const VariantList_t& args)
    {
        printf("onOff\n");
        mainLoop->quit();
    });
    hsm.registerState(TrafficLightState::INITIALIZING, [&hsm](const VariantList_t& args)
    {
        printf("onInitializing\n");
        usleep(1000000);
        hsm.transition(TrafficLightEvent::INIT_DONE);
    });
    hsm.registerState(TrafficLightState::RED, [&hsm](const VariantList_t& args)
    {
        static int iteration = 0;

        printf("onRed\n");
        usleep(1000000);
        hsm.transition(iteration < 2 ? TrafficLightEvent::NEXT_STATE : TrafficLightEvent::POWER_OFF);
        ++iteration;
    });
    hsm.registerState(TrafficLightState::YELLOW, [&hsm](const VariantList_t& args)
    {
        printf("onYellow\n");
        usleep(1000000);
        hsm.transition(TrafficLightEvent::NEXT_STATE);
    });
    hsm.registerState(TrafficLightState::GREEN, [&hsm](const VariantList_t& args)
    {
        printf("onGreen\n");
        usleep(1000000);
        hsm.transition(TrafficLightEvent::NEXT_STATE);
    });

    hsm.registerSubstateEntryPoint(TrafficLightState::OPERABLE, TrafficLightState::INITIALIZING);
    hsm.registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::RED);
    hsm.registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::YELLOW);
    hsm.registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::GREEN);

    hsm.registerTransition(TrafficLightState::OFF, TrafficLightState::OPERABLE, TrafficLightEvent::POWER_ON);
    hsm.registerTransition(TrafficLightState::OPERABLE, TrafficLightState::OFF, TrafficLightEvent::POWER_OFF);
    hsm.registerTransition(TrafficLightState::INITIALIZING, TrafficLightState::RED, TrafficLightEvent::INIT_DONE);
    hsm.registerTransition(TrafficLightState::RED, TrafficLightState::YELLOW, TrafficLightEvent::NEXT_STATE);
    hsm.registerTransition(TrafficLightState::YELLOW, TrafficLightState::GREEN, TrafficLightEvent::NEXT_STATE);
    hsm.registerTransition(TrafficLightState::GREEN, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE);

    hsm.transition(TrafficLightEvent::POWER_ON);

    mainLoop->run();

    return 0;
}