#include <chrono>
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>
#include <hsmcpp/hsm.hpp>
#include <thread>

namespace States {
    const hsmcpp::StateID_t IDLE = 0;
    const hsmcpp::StateID_t OFF = 1;
    const hsmcpp::StateID_t ON = 2;
}

namespace Events {
    const hsmcpp::EventID_t START = 0;
    const hsmcpp::EventID_t ON_TIMER1 = 1;
}

namespace Timers {
    const hsmcpp::TimerID_t TIMER1 = 1;
}

using TimersHSM_t = hsmcpp::HierarchicalStateMachine;

int main(const int argc, const char** argv) {
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm> dispatcher = hsmcpp::HsmEventDispatcherGLibmm::create();
    hsmcpp::HierarchicalStateMachine hsm(States::IDLE);

    hsm.registerState(States::IDLE, [&hsm](const hsmcpp::VariantVector_t& args) { printf("Idle\n"); });
    hsm.registerState(States::OFF, [&hsm](const hsmcpp::VariantVector_t& args) { printf("Off\n"); });
    hsm.registerState(States::ON, [&hsm](const hsmcpp::VariantVector_t& args) { printf("On\n"); });

    hsm.registerTransition(States::IDLE, States::OFF, Events::START);
    hsm.registerTransition(States::OFF, States::ON, Events::ON_TIMER1);
    hsm.registerTransition(States::ON, States::OFF, Events::ON_TIMER1);

    hsm.registerTimer(Timers::TIMER1, Events::ON_TIMER1);

    hsm.registerStateAction(States::IDLE,
                            TimersHSM_t::StateActionTrigger::ON_STATE_EXIT,
                            TimersHSM_t::StateAction::START_TIMER,
                            Timers::TIMER1,
                            1000,
                            false);

    hsm.initialize(dispatcher);

    hsm.transition(Events::START);
    mainLoop->run();

    return 0;
}