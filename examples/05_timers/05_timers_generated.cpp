#include "gen/TimerHsmBase.hpp"
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>

using namespace std::chrono_literals;
using namespace hsmcpp;

class TimerHsm: public TimerHsmBase
{
public:
    virtual ~TimerHsm(){}

// HSM state changed callbacks
protected:
    void On(const hsmcpp::VariantList_t& args) override
    {
        printf("On\n");
    }

    void Off(const hsmcpp::VariantList_t& args) override
    {
        printf("Off\n");
    }
};

int main(const int argc, const char**argv)
{
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> mainLoop = Glib::MainLoop::create();
    TimerHsm hsm;

    hsm.enableHsmDebugging();
    hsm.initialize(std::make_shared<HsmEventDispatcherGLibmm>());

    hsm.transition(TimerHsmEvents::START);
    mainLoop->run();

    return 0;
}