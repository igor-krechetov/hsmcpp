#include <chrono>
#include <thread>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

using namespace std::chrono_literals;
using namespace hsmcpp;

enum class States
{
    OFF,
    ON
};

enum class Events
{
    SWITCH
};

int main(const int argc, const char**argv)
{
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.registerState(States::OFF, [&hsm](const VariantVector_t& args)
    {
        printf("Off\n");
        std::this_thread::sleep_for(1000ms);
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantVector_t& args)
    {
        printf("On\n");
        std::this_thread::sleep_for(1000ms);
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());
    hsm.transition(Events::SWITCH);
    mainLoop->run();

    return 0;
}