#include <unistd.h>

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"
#include "hsmcpp/hsm.hpp"

using namespace hsmcpp;

enum class TrafficLightState {
    OFF,

    OPERABLE,
    INITIALIZING,
    RED,
    YELLOW,
    GREEN
};

enum class TrafficLightEvent { POWER_ON, POWER_OFF, INIT_DONE, NEXT_STATE };

class TrafficLight : public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent> {
public:
    TrafficLight(Glib::RefPtr<Glib::MainLoop> loop)
        : HierarchicalStateMachine(TrafficLightState::OFF)
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

        initialize(std::make_shared<HsmEventDispatcherGLibmm>());
    }

    void onOff(const VariantVector_t& args) {
        printf("onOff\n");
        mLoop->quit();
    }

    void onInitializing(const VariantVector_t& args) {
        printf("onInitializing\n");
        usleep(1000000);
        transition(TrafficLightEvent::INIT_DONE);
    }

    void onRed(const VariantVector_t& args) {
        static int iteration = 0;

        printf("onRed\n");
        usleep(1000000);
        transition(iteration < 2 ? TrafficLightEvent::NEXT_STATE : TrafficLightEvent::POWER_OFF);
        ++iteration;
    }

    void onYellow(const VariantVector_t& args) {
        printf("onYellow\n");
        usleep(1000000);
        transition(TrafficLightEvent::NEXT_STATE);
    }

    void onGreen(const VariantVector_t& args) {
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
    TrafficLight hsm(mainLoop);

    hsm.transition(TrafficLightEvent::POWER_ON);

    mainLoop->run();
    return 0;
}