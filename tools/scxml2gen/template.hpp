// Content of this file was generated

#ifndef __GEN_HSM_%CLASS_NAME%__
#define __GEN_HSM_%CLASS_NAME%__

#include <hsmcpp/hsm.hpp>

enum class @ENUM_STATES@
{
~~~BLOCK:ENUM_STATES_ITEM~~~
    @ENUM_STATES_ITEM@,
~~~BLOCK_END~~~
};

enum class @ENUM_EVENTS@
{
~~~BLOCK:ENUM_EVENTS_ITEM~~~
    @ENUM_EVENTS_ITEM@,
~~~BLOCK_END~~~
};

class @CLASS_NAME@: public hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>
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

protected:
    std::string getStateName(const @ENUM_STATES@ state) override;
    std::string getEventName(const @ENUM_EVENTS@ event) override;
};

#endif // __GEN_HSM_%CLASS_NAME%__