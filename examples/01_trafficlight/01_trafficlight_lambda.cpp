#include <unistd.h>

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"
#include "hsmcpp/hsm.hpp"

namespace TrafficLightState {
    const hsmcpp::StateID_t OFF = 0;

    const hsmcpp::StateID_t OPERABLE = 1;
    const hsmcpp::StateID_t INITIALIZING = 2;
    const hsmcpp::StateID_t RED = 3;
    const hsmcpp::StateID_t YELLOW = 4;
    const hsmcpp::StateID_t GREEN = 5;
}  // namespace TrafficLightState

namespace TrafficLightEvent {
    const hsmcpp::EventID_t POWER_ON = 0;
    const hsmcpp::EventID_t POWER_OFF = 1;
    const hsmcpp::EventID_t INIT_DONE = 2;
    const hsmcpp::EventID_t NEXT_STATE = 3;
}  // namespace TrafficLightEvent

int main(const int argc, const char** argv) {
    Glib::init();

    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    hsmcpp::HierarchicalStateMachine hsm(TrafficLightState::OFF);

    hsm.registerState(TrafficLightState::OFF, [mainLoop](const hsmcpp::VariantVector_t& args) {
        printf("onOff\n");
        mainLoop->quit();
    });
    hsm.registerState(TrafficLightState::INITIALIZING, [&hsm](const hsmcpp::VariantVector_t& args) {
        printf("onInitializing\n");
        usleep(1000000);
        hsm.transition(TrafficLightEvent::INIT_DONE);
    });
    hsm.registerState(TrafficLightState::RED, [&hsm](const hsmcpp::VariantVector_t& args) {
        static int iteration = 0;

        printf("onRed\n");
        usleep(1000000);
        hsm.transition(iteration < 2 ? TrafficLightEvent::NEXT_STATE : TrafficLightEvent::POWER_OFF);
        ++iteration;
    });
    hsm.registerState(TrafficLightState::YELLOW, [&hsm](const hsmcpp::VariantVector_t& args) {
        printf("onYellow\n");
        usleep(1000000);
        hsm.transition(TrafficLightEvent::NEXT_STATE);
    });
    hsm.registerState(TrafficLightState::GREEN, [&hsm](const hsmcpp::VariantVector_t& args) {
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

    hsm.initialize(std::make_shared<hsmcpp::HsmEventDispatcherGLibmm>());
    hsm.transition(TrafficLightEvent::POWER_ON);

    mainLoop->run();

    return 0;
}