// Content of this file was generated

#include "@HPP_FILE@"

@CLASS_NAME@::@CLASS_NAME@()
    : hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>(@ENUM_STATES@::@INITIAL_STATE@) {
}

bool @CLASS_NAME@::initialize(const std::shared_ptr<hsmcpp::IHsmEventDispatcher>& dispatcher) {
    configureHsm();
    return hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::initialize(dispatcher);
}

void @CLASS_NAME@::configureHsm() {
    registerFailedTransitionCallback<@CLASS_NAME@>(this, &@CLASS_NAME@::onTransitionFailed);
    configureStates();
    configureSubstates();
    configureTransitions();
    configureTimers();
    configureActions();
}

void @CLASS_NAME@::configureStates() {
    @REGISTER_STATES@
}

void @CLASS_NAME@::configureSubstates() {
    @REGISTER_SUBSTATES@
}

void @CLASS_NAME@::configureTransitions() {
    @REGISTER_TRANSITIONS@
}

void @CLASS_NAME@::configureTimers() {
    @REGISTER_TIMERS@
}

void @CLASS_NAME@::configureActions() {
    @REGISTER_ACTIONS@
}

void @CLASS_NAME@::onTransitionFailed(const @ENUM_EVENTS@ event, const hsmcpp::VariantVector_t& args) {
    // do nothing
}

std::string @CLASS_NAME@::getStateName(const @ENUM_STATES@ state) const {
    std::string stateName;

#ifndef HSM_DISABLE_TRACES
    switch(state) {
~~~BLOCK:ENUM_STATES_ITEM~~~
        case @ENUM_STATES@::@ENUM_STATES_ITEM@:
            stateName = "@ENUM_STATES_ITEM@";
            break;
~~~BLOCK_END~~~
        default:
            stateName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getStateName(state);
            break;
    }
#else
    stateName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getStateName(state);
#endif // HSM_DISABLE_TRACES

    return stateName;
}

std::string @CLASS_NAME@::getEventName(const @ENUM_EVENTS@ event) const {
    std::string eventName;

#ifndef HSM_DISABLE_TRACES
    switch(event) {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
        case @ENUM_EVENTS@::@ENUM_EVENTS_ITEM@:
            eventName = "@ENUM_EVENTS_ITEM@";
            break;
~~~BLOCK_END~~~
        default:
            eventName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getEventName(event);
            break;
    }
#else
    eventName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getEventName(event);
#endif // HSM_DISABLE_TRACES

    return eventName;
}