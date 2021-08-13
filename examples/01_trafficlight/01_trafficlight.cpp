// NOTE: For internal testing and will be removed later

#include "hsmcpp/hsm.hpp"
#include <thread>
#include "hsmcpp/logging.hpp"
#include "hsmcpp/HsmEventDispatcherGLibmm.hpp"

#undef __HSM_TRACE_CLASS__
#define __HSM_TRACE_CLASS__                         "01_trafficlight"

__HSM_TRACE_PREINIT__();

using namespace std::chrono_literals;
using namespace hsmcpp;

enum class TrafficLightState
{
    OFF,
    STARTING,
    RED,
    YELLOW,
    GREEN
};

enum class TrafficLightEvent
{
    TURN_ON,
    TURN_OFF,
    NEXT_STATE
};

class TrafficLight: public HierarchicalStateMachine<TrafficLightState, TrafficLightEvent>
{
public:
    TrafficLight() : HierarchicalStateMachine(TrafficLightState::OFF)
    {
        registerState<TrafficLight>(TrafficLightState::OFF, this, &TrafficLight::onOff, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::STARTING, this, &TrafficLight::onStarting, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::RED, this, &TrafficLight::onRed, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::YELLOW, this, &TrafficLight::onYellow, nullptr, nullptr);
        registerState<TrafficLight>(TrafficLightState::GREEN, this, &TrafficLight::onGreen, nullptr, nullptr);

        registerTransition(TrafficLightState::OFF, TrafficLightState::STARTING, TrafficLightEvent::TURN_ON, nullptr, nullptr);
        registerTransition(TrafficLightState::STARTING, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE, this, &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::RED, TrafficLightState::YELLOW, TrafficLightEvent::NEXT_STATE, this, &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::YELLOW, TrafficLightState::GREEN, TrafficLightEvent::NEXT_STATE, this, &TrafficLight::onNextStateTransition);
        registerTransition(TrafficLightState::GREEN, TrafficLightState::RED, TrafficLightEvent::NEXT_STATE, this, &TrafficLight::onNextStateTransition);

        initialize(std::make_shared<HsmEventDispatcherGLibmm>());
    }

    void onNextStateTransition(const VariantList_t& args)
    {
        if (args.size() == 2)
        {
            printf("----> onNextStateTransition (args=%d, thread=%d, index=%d)\n", (int)args.size(), (int)args[0].toInt64(), (int)args[1].toInt64());
        }
        else
        {
            printf("----> onNextStateTransition (no args)\n");
        }

        std::this_thread::sleep_for(1000ms);
    }

    void onOff(const VariantList_t& args){ printf("----> OFF\n"); }
    void onStarting(const VariantList_t& args){ printf("----> onStarting\n"); }
    void onRed(const VariantList_t& args){ printf("----> onRed\n"); }
    void onYellow(const VariantList_t& args){ printf("----> onYellow\n"); }
    void onGreen(const VariantList_t& args){ printf("----> onGreen\n"); }
};

Glib::RefPtr<Glib::MainLoop> mMainLoop;
TrafficLight* tl = nullptr;

void simulate()
{
    int index = 0;

    printf("[T0] wait 2000 ms...\n");
    std::this_thread::sleep_for(2000ms);
    printf("[T0] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(2000ms);

    while(true)
    {
        tl->transition(TrafficLightEvent::NEXT_STATE);
        index++;
        std::this_thread::sleep_for(1000ms);
    }
}

void simulateSync1()
{
    int index = 0;

    printf("[T1] wait 2000 ms...\n");
    std::this_thread::sleep_for(2000ms);
    printf("[T1] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(2000ms);

    while(true)
    {
        bool status;

        printf("[T1] BEFORE transition\n");
        status = tl->transitionEx(TrafficLightEvent::NEXT_STATE, true, true, HSM_WAIT_INDEFINITELY, 1, index);
        printf("[T1] AFTER transition: %d\n", (int)status);
        index++;
        std::this_thread::sleep_for(3000ms);
    }
}

void simulateSync2()
{
    int index = 0;

    printf("[T2] wait 2000 ms...\n");
    std::this_thread::sleep_for(2000ms);
    printf("[T2] starting work...\n");

    tl->transition(TrafficLightEvent::TURN_ON);
    std::this_thread::sleep_for(2000ms);

    while(true)
    {
        bool status;

        printf("[T2] BEFORE transition\n");
        status = tl->transitionEx(TrafficLightEvent::NEXT_STATE, true, true, HSM_WAIT_INDEFINITELY, 2, index);
        printf("[T2] AFTER transition: %d\n", (int)status);
        index++;
        std::this_thread::sleep_for(3000ms);
    }
}

int main(const int argc, const char**argv)
{
    __HSM_TRACE_INIT__();
    __HSM_TRACE_CALL_ARGS__("01_trafficlight");

    Glib::init();
    mMainLoop = Glib::MainLoop::create();
    tl = new TrafficLight();

    std::thread threadSimulate0(simulate);
    std::thread threadSimulate1(simulateSync1);
    std::thread threadSimulate2(simulateSync2);

    mMainLoop->run();

    delete tl;
    tl = nullptr;

    return 0;
}