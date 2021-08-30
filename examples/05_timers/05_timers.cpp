#include <chrono>
#include <thread>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

using namespace std::chrono_literals;
using namespace hsmcpp;

enum class States
{
    IDLE,
    OFF,
    ON
};

enum class Events
{
    START,
    ON_TIMER1
};

enum class Timers
{
    TIMER1 = 1
};

using TimersHSM_t = HierarchicalStateMachine<States, Events>;

int main(const int argc, const char**argv)
{
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    HierarchicalStateMachine<States, Events> hsm(States::IDLE);

    hsm.registerState(States::IDLE, [&hsm](const VariantVector_t& args)
    {
        printf("Idle\n");
    });
    hsm.registerState(States::OFF, [&hsm](const VariantVector_t& args)
    {
        printf("Off\n");
    });
    hsm.registerState(States::ON, [&hsm](const VariantVector_t& args)
    {
        printf("On\n");
    });

    hsm.registerTransition(States::IDLE, States::OFF, Events::START);
    hsm.registerTransition(States::OFF, States::ON, Events::ON_TIMER1);
    hsm.registerTransition(States::ON, States::OFF, Events::ON_TIMER1);

    hsm.registerTimer(static_cast<int>(Timers::TIMER1), Events::ON_TIMER1);

    hsm.registerStateAction(States::IDLE,
                            TimersHSM_t::StateActionTrigger::ON_STATE_EXIT,
                            TimersHSM_t::StateAction::START_TIMER,
                            static_cast<int>(Timers::TIMER1),
                            1000,
                            false);

    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());

    hsm.transition(Events::START);
    mainLoop->run();

    return 0;
}