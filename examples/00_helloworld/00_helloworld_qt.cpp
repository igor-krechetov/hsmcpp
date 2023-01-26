#include <chrono>
#include <thread>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherQt.hpp>
#include <QCoreApplication>

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

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.registerState(States::OFF, [&hsm](const VariantVector_t& args)
    {
        (void)printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantVector_t& args)
    {
        (void)printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    if (true == hsm.initialize(std::make_shared<HsmEventDispatcherQt>())) {
        hsm.transition(Events::SWITCH);

        app.exec();
    }

    return 0;
}