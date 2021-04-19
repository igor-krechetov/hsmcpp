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

protected:
    std::string getStateName(const DebugTestHsmStates state) override
    {
        std::string stateName;

        switch(state)
        {
            case DebugTestHsmStates::state_1:
                stateName ="state_1";
                break;
            case DebugTestHsmStates::parent_1:
                stateName = "parent_1";
                break;
            case DebugTestHsmStates::state_2:
                stateName = "state_2";
                break;
            case DebugTestHsmStates::parent_2:
                stateName = "parent_2";
                break;
            case DebugTestHsmStates::state_3:
                stateName = "state_3";
                break;
            case DebugTestHsmStates::state_4:
                stateName = "state_4";
                break;
            case DebugTestHsmStates::state_5:
                stateName = "state_5";
                break;
            case DebugTestHsmStates::state_6:
                stateName = "state_6";
                break;
            case DebugTestHsmStates::state_7:
                stateName = "state_7";
                break;
            case DebugTestHsmStates::state_8:
                stateName = "state_8";
                break;
            default:
                stateName = DebugTestHsmBase::getStateName(state);
                break;
        }

        return stateName;
    }

    virtual std::string getEventName(const DebugTestHsmEvents event)
    {
        std::string eventName;

        switch(event)
        {
            case DebugTestHsmEvents::event_next_parent:
                eventName = "event_next_parent";
                break;
            case DebugTestHsmEvents::event_self:
                eventName = "event_self";
                break;
            case DebugTestHsmEvents::event_next:
                eventName = "event_next";
                break;
            case DebugTestHsmEvents::event_exit_parent:
                eventName = "event_exit_parent";
                break;
            default:
                eventName = DebugTestHsmBase::getEventName(event);
                break;
        }

        return eventName;
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