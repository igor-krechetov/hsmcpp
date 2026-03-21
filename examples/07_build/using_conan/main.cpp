#include <chrono>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <hsmcpp/hsm.hpp>
#include <thread>

namespace States {
const hsmcpp::StateID_t OFF = 0;
const hsmcpp::StateID_t ON = 1;
}  // namespace States

namespace Events {
const hsmcpp::EventID_t SWITCH = 0;
}  // namespace Events

int main(const int argc, const char** argv) {
    (void)argc;
    (void)argv;

    std::shared_ptr<hsmcpp::HsmEventDispatcherSTD> dispatcher = hsmcpp::HsmEventDispatcherSTD::create();
    hsmcpp::HierarchicalStateMachine hsm(States::OFF);

    hsm.registerState(States::OFF, [&hsm](const hsmcpp::VariantVector_t& args) {
        (void)args;
        (void)printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerState(States::ON, [&hsm](const hsmcpp::VariantVector_t& args) {
        (void)args;
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
