#include "gen/ActionsHsmBase.hpp"
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

using namespace hsmcpp;

class ActionsHsm: public ActionsHsmBase
{
public:
    virtual ~ActionsHsm(){}

// HSM state changed callbacks
protected:
    void On(const hsmcpp::VariantVector_t& args) override
    {
        printf("On\n");
    }

    void Off(const hsmcpp::VariantVector_t& args) override
    {
        printf("Off\n");
    }

    void onNextStep(const hsmcpp::VariantVector_t& args) override
    {
        printf("onNextStep: <%s>, %ld\n", args[0].toString().c_str(), args[1].toInt64());
    }

    bool isTransitionAllowed(const VariantVector_t&) override
    {
        return true;
    }
};

int main(const int argc, const char**argv)
{
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    ActionsHsm hsm;

    hsm.enableHsmDebugging();
    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());

    hsm.transition(ActionsHsmEvents::START);
    mainLoop->run();

    return 0;
}