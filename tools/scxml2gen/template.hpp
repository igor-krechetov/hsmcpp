// Content of this file was generated

#ifndef GEN_HSM_%CLASS_NAME%
#define GEN_HSM_%CLASS_NAME%

#include <hsmcpp/hsm.hpp>

namespace @ENUM_STATES@ {
~~~BLOCK:ENUM_STATES_ITEM~~~
    constexpr hsmcpp::StateID_t @ENUM_STATES_ITEM@ = @BLOCK_ITEM_INDEX@;
~~~BLOCK_END~~~
}

namespace @ENUM_EVENTS@ {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
    constexpr hsmcpp::EventID_t @ENUM_EVENTS_ITEM@ = @BLOCK_ITEM_INDEX@;
~~~BLOCK_END~~~

    // INVALID = INVALID_ID
}

namespace @ENUM_TIMERS@ {
~~~BLOCK:ENUM_TIMERS_ITEM~~~
    constexpr hsmcpp::TimerID_t @ENUM_TIMERS_ITEM@ = @BLOCK_ITEM_INDEX@;
~~~BLOCK_END~~~
}

class @CLASS_NAME@: public hsmcpp::HierarchicalStateMachine {
    using @CLASS_NAME@TransitionCallbackPtr_t           = void (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@TransitionConditionCallbackPtr_t  = bool (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateChangedCallbackPtr_t         = void (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateEnterCallbackPtr_t           = bool (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateExitCallbackPtr_t            = bool (@CLASS_NAME@::*)();
    using @CLASS_NAME@TransitionFailedCallbackPtr_t     = void (@CLASS_NAME@::*)(const hsmcpp::EventID_t, const hsmcpp::VariantVector_t&);

public:
    @CLASS_NAME@();
    virtual ~@CLASS_NAME@() = default;

    bool initialize(const std::weak_ptr<hsmcpp::IHsmEventDispatcher>& dispatcher) override;

protected:
    void configureHsm();
    void configureStates();
    void configureSubstates();
    void configureTransitions();
    void configureTimers();
    void configureActions();

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
    // NOTE: override this method in child class if needed
    virtual void onTransitionFailed(const hsmcpp::EventID_t event, const hsmcpp::VariantVector_t& args);

    @HSM_TRANSITION_ACTIONS@

// HSM transition condition callbacks
protected:
    @HSM_TRANSITION_CONDITIONS@

protected:
    std::string getStateName(const hsmcpp::StateID_t state) const override;
    std::string getEventName(const hsmcpp::EventID_t event) const override;
};

#endif // GEN_HSM_%CLASS_NAME%