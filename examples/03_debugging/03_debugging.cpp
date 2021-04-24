#include <unistd.h>
#include <hsmcpp/HsmEventDispatcherSTD.hpp>
#include "gen/DebugTestHsmBase.hpp"

class DebugTestHsm: public DebugTestHsmBase
{
public:
    virtual ~DebugTestHsm(){}

// HSM state changed callbacks
protected:
    void State1OnState(const VariantList_t& args) override
    {
    }

// HSM state entering callbacks
protected:
    bool State1OnEntry(const VariantList_t& args) override
    {
        return true;
    }

// HSM state exiting callbacks
protected:
    bool State1OnExit() override
    {
        return true;
    }
};

int main(const int argc, const char**argv)
{
    std::shared_ptr<HsmEventDispatcherSTD> dispatcher = std::make_shared<HsmEventDispatcherSTD>();
    DebugTestHsm hsm;
    Variant arg1 = Variant::make(false);
    Variant arg2 = Variant::make("test value");
    Variant arg3 = Variant::make(123.89);
    Variant arg4 = Variant::make(Variant::make(1), Variant::make(2));

    hsm.enableHsmDebugging();
    hsm.initialize(dispatcher);
    hsm.transition(DebugTestHsmEvents::event_next, arg1, arg2, arg3, arg4);
    hsm.transition(DebugTestHsmEvents::event_next);
    hsm.transition(DebugTestHsmEvents::event_self);
    hsm.transition(DebugTestHsmEvents::event_next_parent);
    hsm.transition(DebugTestHsmEvents::event_next);
    hsm.transition(DebugTestHsmEvents::event_exit_parent);
    hsm.transition(DebugTestHsmEvents::event_next, arg1, arg2, arg3, arg4);

    dispatcher->join();

    return 0;
}