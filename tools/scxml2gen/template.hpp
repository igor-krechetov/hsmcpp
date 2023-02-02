// Content of this file was generated

#ifndef GEN_HSM_%CLASS_NAME%
#define GEN_HSM_%CLASS_NAME%

#include <hsmcpp/hsm.hpp>

enum class @ENUM_STATES@ {
~~~BLOCK:ENUM_STATES_ITEM~~~
    @ENUM_STATES_ITEM@,
~~~BLOCK_END~~~
};

enum class @ENUM_EVENTS@ {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
    @ENUM_EVENTS_ITEM@,
~~~BLOCK_END~~~

    INVALID = INVALID_ID
};

enum class @ENUM_TIMERS@ {
~~~BLOCK:ENUM_TIMERS_ITEM~~~
    @ENUM_TIMERS_ITEM@,
~~~BLOCK_END~~~
};

class @CLASS_NAME@: public hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@> {
    using @CLASS_NAME@TransitionCallbackPtr_t           = void (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@TransitionConditionCallbackPtr_t  = bool (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateChangedCallbackPtr_t         = void (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateEnterCallbackPtr_t           = bool (@CLASS_NAME@::*)(const hsmcpp::VariantVector_t&);
    using @CLASS_NAME@StateExitCallbackPtr_t            = bool (@CLASS_NAME@::*)();
    using @CLASS_NAME@TransitionFailedCallbackPtr_t     = void (@CLASS_NAME@::*)(const @ENUM_EVENTS@, const hsmcpp::VariantVector_t&);

public:
    @CLASS_NAME@();
    virtual ~@CLASS_NAME@() = default;

    bool initialize(const std::shared_ptr<hsmcpp::IHsmEventDispatcher>& dispatcher) override;

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
    virtual void onTransitionFailed(const @ENUM_EVENTS@ event, const hsmcpp::VariantVector_t& args);

    @HSM_TRANSITION_ACTIONS@

// HSM transition condition callbacks
protected:
    @HSM_TRANSITION_CONDITIONS@

protected:
    std::string getStateName(const @ENUM_STATES@ state) const override;
    std::string getEventName(const @ENUM_EVENTS@ event) const override;
};

#endif // GEN_HSM_%CLASS_NAME%