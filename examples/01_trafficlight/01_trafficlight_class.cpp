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

class TrafficLight : public hsmcpp::HierarchicalStateMachine {
public:
    TrafficLight(const std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm>& dispatcher, const Glib::RefPtr<Glib::MainLoop>& loop)
        : hsmcpp::HierarchicalStateMachine(TrafficLightState::OFF)
        , mLoop(loop) {
        registerState(TrafficLightState::OFF, this, &TrafficLight::onOff);
        registerState(TrafficLightState::INITIALIZING, this, &TrafficLight::onInitializing);
        registerState(TrafficLightState::RED, this, &TrafficLight::onRed);
        registerState(TrafficLightState::YELLOW, this, &TrafficLight::onYellow);
        registerState(TrafficLightState::GREEN, this, &TrafficLight::onGreen);

        registerSubstateEntryPoint(TrafficLightState::OPERABLE, TrafficLightState::RED);

        registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::RED);
        registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::YELLOW);
        registerSubstate(TrafficLightState::OPERABLE, TrafficLightState::GREEN);

        registerTransition(TrafficLightState::OFF, TrafficLightState::OPERABLE, TrafficLightEvent::POWER_ON);
        registerTransition(TrafficLightState::OPERABLE, TrafficLightState::OFF, TrafficLightEvent::POWER_OFF);
        registerTransition(TrafficLightState::INITIALIZING, TrafficLightState::RED, TrafficLightEvent::INIT_DONE);
        registerTransition(TrafficLightState::RED, TrafficLightState::YELLOW, TrafficLightEvent::NEXT_STATE);
        registerTransition(TrafficLightState::YELLOW, TrafficLightState::GREEN, TrafficLightEvent::NEXT_STATE);
        registerTransition(TrafficLightState::GREEN, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE);

        initialize(dispatcher);
    }

    void onOff(const hsmcpp::VariantVector_t& args) {
        printf("onOff\n");
        mLoop->quit();
    }

    void onInitializing(const hsmcpp::VariantVector_t& args) {
        printf("onInitializing\n");
        usleep(1000000);
        transition(TrafficLightEvent::INIT_DONE);
    }

    void onRed(const hsmcpp::VariantVector_t& args) {
        static int iteration = 0;

        printf("onRed\n");
        usleep(1000000);
        transition(iteration < 2 ? TrafficLightEvent::NEXT_STATE : TrafficLightEvent::POWER_OFF);
        ++iteration;
    }

    void onYellow(const hsmcpp::VariantVector_t& args) {
        printf("onYellow\n");
        usleep(1000000);
        transition(TrafficLightEvent::NEXT_STATE);
    }

    void onGreen(const hsmcpp::VariantVector_t& args) {
        printf("onGreen\n");
        usleep(1000000);
        transition(TrafficLightEvent::NEXT_STATE);
    }

private:
    Glib::RefPtr<Glib::MainLoop> mLoop;
};

int main(const int argc, const char** argv) {
    Glib::init();

    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm> dispatcher = hsmcpp::HsmEventDispatcherGLibmm::create();
    TrafficLight hsm(dispatcher, mainLoop);

    hsm.transition(TrafficLightEvent::POWER_ON);

    mainLoop->run();
    return 0;
}