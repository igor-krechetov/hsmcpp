// Content of this file was generated

#ifndef __GEN_HSM_%CLASS_NAME%__
#define __GEN_HSM_%CLASS_NAME%__

#include <hsmcpp/hsm.hpp>

enum class @ENUM_STATES@
{
    @ENUM_STATES_DEF@
};

enum class @ENUM_EVENTS@
{
    @ENUM_EVENTS_DEF@
};

class @CLASS_NAME@: public HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>
{
public:
    @CLASS_NAME@();
    virtual ~@CLASS_NAME@();

protected:
    void configureHsm();

// HSM state changed callbacks
protected:
    @HSM_STATE_ACTIONS@

// HSM state entering callbacks
protected:
    @HSM_STATE_ENTERING_ACTIONS@

// HSM state exiting callbacks
protected:
    @HSM_STATE_EXITING_ACTIONS@

// HSM transition callbacks
protected:
    @HSM_TRANSITION_ACTIONS@

// HSM transition condition callbacks
protected:
    @HSM_TRANSITION_CONDITIONS@
};

#endif // __GEN_HSM_%CLASS_NAME%__