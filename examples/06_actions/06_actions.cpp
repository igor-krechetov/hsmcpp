#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

#include "gen/ActionsHsmBase.hpp"

using namespace hsmcpp;

class ActionsHsm : public ActionsHsmBase {
public:
    virtual ~ActionsHsm() {}

    // HSM state changed callbacks
protected:
    void On(const hsmcpp::VariantVector_t& args) override {
        printf("On\n");
    }

    void Off(const hsmcpp::VariantVector_t& args) override {
        printf("Off\n");
    }

    void onNextStep(const hsmcpp::VariantVector_t& args) override {
        printf("onNextStep: <%s>, %ld\n", args[0].toString().c_str(), args[1].toInt64());
    }

    bool isTransitionAllowed(const VariantVector_t&) override {
        return true;
    }
};

int main(const int argc, const char** argv) {
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    std::shared_ptr<hsmcpp::HsmEventDispatcherGLibmm> dispatcher = hsmcpp::HsmEventDispatcherGLibmm::create();
    ActionsHsm hsm;

    hsm.enableHsmDebugging();
    hsm.initialize(dispatcher);

    hsm.transition(ActionsHsmEvents::START);
    mainLoop->run();

    return 0;
}