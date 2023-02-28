#include <chrono>
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>
#include <hsmcpp/hsm.hpp>
#include <thread>

using namespace hsmcpp;

namespace States {
    const hsmcpp::StateID_t OFF = 0;
    const hsmcpp::StateID_t ON = 1;
}

namespace Events {
    const hsmcpp::EventID_t SWITCH = 0;
}

int main(const int argc, const char** argv) {
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    HierarchicalStateMachine hsm(States::OFF);

    hsm.registerState(States::OFF, [&hsm](const hsmcpp::VariantVector_t& args) {
        printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });
    hsm.registerState(States::ON, [&hsm](const hsmcpp::VariantVector_t& args) {
        printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        hsm.transition(Events::SWITCH);
    });

    hsm.registerTransition(States::OFF, States::ON, Events::SWITCH);
    hsm.registerTransition(States::ON, States::OFF, Events::SWITCH);

    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());
    hsm.transition(Events::SWITCH);
    mainLoop->run();

    return 0;
}