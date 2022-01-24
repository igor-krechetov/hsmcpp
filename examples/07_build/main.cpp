#include <chrono>
#include <thread>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include <hsmcpp/HsmEventDispatcherGLibmm.hpp>
#include "gen/SwitchHsmBase.hpp"

using namespace hsmcpp;

class SwitchHsm: public SwitchHsmBase
{
public:
    virtual ~SwitchHsm(){}

// HSM state changed callbacks
protected:
    void onOff(const VariantVector_t& args) override
    {
        printf("Off\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(SwitchHsmEvents::SWITCH);
    }

    void onOn(const VariantVector_t& args) override
    {
        printf("On\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        transition(SwitchHsmEvents::SWITCH);
    }
};

int main(const int argc, const char**argv)
{
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    // NOTE: Second dispatched is only for build testing.
    //       In real situation one type of dispatcher is usually enough.
    std::shared_ptr<HsmEventDispatcherGLibmm> dispatcher2 = std::make_shared<HsmEventDispatcherGLibmm>();
    SwitchHsm hsm;

    hsm.initialize(dispatcher);
    hsm.transition(SwitchHsmEvents::SWITCH);

    dispatcher->join();

    return 0;
}