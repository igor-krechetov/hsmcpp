#include <unistd.h>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/SwitchHsmBase.hpp"

class SwitchHsm: public SwitchHsmBase
{
public:
    virtual ~SwitchHsm(){}

// HSM state changed callbacks
protected:
    void onOff(const VariantList_t& args) override
    {
        printf("Off\n");
        usleep(1000000);
        transition(SwitchHsmEvents::SWITCH);
    }

    void onOn(const VariantList_t& args) override
    {
        printf("On\n");
        usleep(1000000);
        transition(SwitchHsmEvents::SWITCH);
    }
};

int main(const int argc, const char**argv)
{
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    SwitchHsm hsm;

    hsm.initialize(dispatcher);
    hsm.transition(SwitchHsmEvents::SWITCH);

    dispatcher->join();

    return 0;
}