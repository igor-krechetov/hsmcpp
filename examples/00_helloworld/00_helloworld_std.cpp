#include <chrono>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <hsmcpp/hsm.hpp>
#include <thread>

using namespace hsmcpp;

enum class States { OFF, ON };

enum class Events { SWITCH };

int main(const int argc, const char** argv) {
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    HierarchicalStateMachine<States, Events> hsm(States::OFF);

    hsm.registerState(States::OFF, [&hsm](const VariantVector_t& args) {
        (void)printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const VariantVector_t& args) {
        (void)printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    if (true == hsm.initialize(dispatcher)) {
        hsm.transition(Events::SWITCH);
        dispatcher->join();
    }

    return 0;
}