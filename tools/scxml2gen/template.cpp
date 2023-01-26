// Content of this file was generated

#include "@HPP_FILE@"

@CLASS_NAME@::@CLASS_NAME@()
    : hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>(@ENUM_STATES@::@INITIAL_STATE@)
{
}

@CLASS_NAME@::~@CLASS_NAME@()
{}

bool @CLASS_NAME@::initialize(const std::shared_ptr<hsmcpp::IHsmEventDispatcher>& dispatcher) {
    configureHsm();
    return hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::initialize(dispatcher);
}

void @CLASS_NAME@::configureHsm() {
    registerFailedTransitionCallback<@CLASS_NAME@>(this, &@CLASS_NAME@::onTransitionFailed);

    @REGISTER_STATES@

    @REGISTER_SUBSTATES@

    @REGISTER_TRANSITIONS@

    @REGISTER_TIMERS@

    @REGISTER_ACTIONS@
}

void @CLASS_NAME@::onTransitionFailed(const @ENUM_EVENTS@ event, const hsmcpp::VariantVector_t& args)
{
    // do nothing
}

std::string @CLASS_NAME@::getStateName(const @ENUM_STATES@ state) const
{
    std::string stateName;

    switch(state)
    {
~~~BLOCK:ENUM_STATES_ITEM~~~
        case @ENUM_STATES@::@ENUM_STATES_ITEM@:
            (void)stateName.assign("@ENUM_STATES_ITEM@");
            break;
~~~BLOCK_END~~~
        default:
            stateName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getStateName(state);
            break;
    }

    return stateName;
}

std::string @CLASS_NAME@::getEventName(const @ENUM_EVENTS@ event) const
{
    std::string eventName;

    switch(event)
    {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
        case @ENUM_EVENTS@::@ENUM_EVENTS_ITEM@:
            (void)eventName.assign("@ENUM_EVENTS_ITEM@");
            break;
~~~BLOCK_END~~~
        default:
            eventName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getEventName(event);
            break;
    }

    return eventName;
}