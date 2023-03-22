#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

#include "gen/TimerHsmBase.hpp"

using namespace hsmcpp;

class TimerHsm : public TimerHsmBase {
public:
    virtual ~TimerHsm() {}

    // HSM state changed callbacks
protected:
    void On(const hsmcpp::VariantVector_t& args) override {
        printf("On\n");
    }

    void Off(const hsmcpp::VariantVector_t& args) override {
        printf("Off\n");
    }
};

int main(const int argc, const char** argv) {
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm> dispatcher = hsmcpp::HsmEventDispatcherGLibmm::create();
    TimerHsm hsm;

    hsm.enableHsmDebugging();
    hsm.initialize(dispatcher);

    hsm.transition(TimerHsmEvents::START);
    mainLoop->run();

    return 0;
}