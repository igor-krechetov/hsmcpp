#include <unistd.h>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>

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
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.initialize(dispatcher);

    hsm.registerState(States::OFF, [&hsm](const VariantList_t& args)
    {
        printf("Off\n");
        usleep(1000000);
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantList_t& args)
    {
        printf("On\n");
        usleep(1000000);
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.transition(Events::SWITCH);

    dispatcher->join();

    return 0;
}