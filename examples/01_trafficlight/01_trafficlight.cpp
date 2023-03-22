// NOTE: For internal testing and will be removed later

#include <thread>

#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"
#include "hsmcpp/hsm.hpp"
#include "hsmcpp/logging.hpp"

#undef HSM_TRACE_CLASS
#define HSM_TRACE_CLASS "01_trafficlight"

using namespace hsmcpp;

namespace TrafficLightState {
    const hsmcpp::StateID_t OFF = 0;
    const hsmcpp::StateID_t STARTING = 1;
    const hsmcpp::StateID_t RED = 2;
    const hsmcpp::StateID_t YELLOW = 3;
    const hsmcpp::StateID_t GREEN = 4;
}  // namespace TrafficLightState

namespace TrafficLightEvent {
    const hsmcpp::EventID_t TURN_ON = 0;
    const hsmcpp::EventID_t TURN_OFF = 1;
    const hsmcpp::EventID_t NEXT_STATE = 2;
}  // namespace TrafficLightEvent

class TrafficLight : public hsmcpp::HierarchicalStateMachine {
public:
    TrafficLight(const std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm>& dispatcher)
        : hsmcpp::HierarchicalStateMachine(TrafficLightState::OFF) {
        registerState<TrafficLight>(TrafficLightState::OFF, this, &TrafficLight::onOff, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::STARTING, this, &TrafficLight::onStarting, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::RED, this, &TrafficLight::onRed, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::YELLOW, this, &TrafficLight::onYellow, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::GREEN, this, &TrafficLight::onGreen, nullptr, nullptr);

        registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);
        registerTransition(TrafficLightState::STARTING,
                           TrafficLightState::RED,
                           TrafficLightEvent::NEXT_STATE,
                           this,
                           &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::RED,
                           TrafficLightState::YELLOW,
                           TrafficLightEvent::NEXT_STATE,
                           this,
                           &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::YELLOW,
                           TrafficLightState::GREEN,
                           TrafficLightEvent::NEXT_STATE,
                           this,
                           &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::GREEN,
                           TrafficLightState::RED,
                           TrafficLightEvent::NEXT_STATE,
                           this,
                           &TrafficLight::onNextStateTransition);

        initialize(dispatcher);
    }

    void onNextStateTransition(const hsmcpp::VariantVector_t& args) {
        if (args.size() == 2) {
            printf("----> onNextStateTransition (args=%d, thread=%d, index=%d)\n",
                   (int)args.size(),
                   (int)args[0].toInt64(),
                   (int)args[1].toInt64());
        } else {
            printf("----> onNextStateTransition (no args)\n");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    void onOff(const hsmcpp::VariantVector_t& args) {
        printf("----> OFF\n");
    }
    void onStarting(const hsmcpp::VariantVector_t& args) {
        printf("----> onStarting\n");
    }
    void onRed(const hsmcpp::VariantVector_t& args) {
        printf("----> onRed\n");
    }
    void onYellow(const hsmcpp::VariantVector_t& args) {
        printf("----> onYellow\n");
    }
    void onGreen(const hsmcpp::VariantVector_t& args) {
        printf("----> onGreen\n");
    }
};

Glib::RefPtr<Glib::MainLoop> mMainLoop;
TrafficLight* tl = nullptr;

void simulate() {
    int index = 0;

    printf("[T0] wait 2000 ms...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    printf("[T0] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    while (true) {
        tl->transition(TrafficLightEvent::NEXT_STATE);
        index++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void simulateSync1() {
    int index = 0;

    printf("[T1] wait 2000 ms...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    printf("[T1] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    while (true) {
        bool status;

        printf("[T1] BEFORE transition\n");
        status = tl->transitionEx(TrafficLightEvent::NEXT_STATE, true, true, HSM_WAIT_INDEFINITELY, 1, index);
        printf("[T1] AFTER transition: %d\n", (int)status);
        index++;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}

void simulateSync2() {
    int index = 0;

    printf("[T2] wait 2000 ms...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    printf("[T2] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    while (true) {
        bool status;

        printf("[T2] BEFORE transition\n");
        status = tl->transitionEx(TrafficLightEvent::NEXT_STATE, true, true, HSM_WAIT_INDEFINITELY, 2, index);
        printf("[T2] AFTER transition: %d\n", (int)status);
        index++;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}

int main(const int argc, const char** argv) {
    HSM_TRACE_INIT();
    HSM_TRACE_CALL_ARGS("01_trafficlight");

    Glib::init();
    mMainLoop = Glib::MainLoop::create();
    std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm> dispatcher = hsmcpp::HsmEventDispatcherGLibmm::create();

    tl = new TrafficLight(dispatcher);

    std::thread threadSimulate0(simulate);
    std::thread threadSimulate1(simulateSync1);
    std::thread threadSimulate2(simulateSync2);

    mMainLoop->run();

    delete tl;
    tl = nullptr;

    return 0;
}