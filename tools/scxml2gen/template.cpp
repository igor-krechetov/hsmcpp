// Content of this file was generated

#include "@HPP_FILE@"

@CLASS_NAME@::@CLASS_NAME@()
    : hsmcpp::HierarchicalStateMachine(@ENUM_STATES@::@INITIAL_STATE@) {
}

bool @CLASS_NAME@::initialize(const std::weak_ptr<hsmcpp::IHsmEventDispatcher>& dispatcher) {
    configureHsm();
    return hsmcpp::HierarchicalStateMachine::initialize(dispatcher);
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

void @CLASS_NAME@::onTransitionFailed(const std::list<hsmcpp::StateID_t>& activeStates,
                                      const hsmcpp::EventID_t event,
                                      const hsmcpp::VariantVector_t& args) {
    // do nothing
}

std::string @CLASS_NAME@::getStateName(const hsmcpp::StateID_t state) const {
    std::string stateName;

#ifndef HSM_DISABLE_TRACES
    switch(state) {
~~~BLOCK:ENUM_STATES_ITEM~~~
        case @ENUM_STATES@::@ENUM_STATES_ITEM@:
            (void)stateName.assign("@ENUM_STATES_ITEM@");
            break;
~~~BLOCK_END~~~
        default:
            stateName = hsmcpp::HierarchicalStateMachine::getStateName(state);
            break;
    }
#else
    stateName = hsmcpp::HierarchicalStateMachine::getStateName(state);
#endif // HSM_DISABLE_TRACES

    return stateName;
}

std::string @CLASS_NAME@::getEventName(const hsmcpp::EventID_t event) const {
    std::string eventName;

#ifndef HSM_DISABLE_TRACES
    switch(event) {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
        case @ENUM_EVENTS@::@ENUM_EVENTS_ITEM@:
            (void)eventName.assign("@ENUM_EVENTS_ITEM@");
            break;
~~~BLOCK_END~~~
        default:
            eventName = hsmcpp::HierarchicalStateMachine::getEventName(event);
            break;
    }
#else
    eventName = hsmcpp::HierarchicalStateMachine::getEventName(event);
#endif // HSM_DISABLE_TRACES

    return eventName;
}