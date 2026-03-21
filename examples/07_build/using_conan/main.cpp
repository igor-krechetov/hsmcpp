#include <chrono>
#include <thread>
#include <hsmcpp/hsm.hpp>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>

using namespace hsmcpp;

enum class States {
    OFF,
    ON,
};

enum class Events {
    SWITCH,
};

int main(const int argc, const char** argv) {
    (void)argc;
    (void)argv;

    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = HsmEventDispatcherSTD::create();
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.initialize(dispatcher);

    hsm.registerState(States::OFF, [&hsm](const VariantList_t&) {
        (void)printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerState(States::ON, [&hsm](const VariantList_t&) {
        (void)printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.transition(Events::SWITCH);

    dispatcher->join();

    return 0;
}
