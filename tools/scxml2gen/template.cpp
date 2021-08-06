// Content of this file was generated

#include "@HPP_FILE@"

@CLASS_NAME@::@CLASS_NAME@()
    : hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>(@ENUM_STATES@::@INITIAL_STATE@)
{
    configureHsm();
}

@CLASS_NAME@::~@CLASS_NAME@()
{}

void @CLASS_NAME@::configureHsm()
{
    @REGISTER_STATES@

    @REGISTER_SUBSTATES@

    @REGISTER_TRANSITIONS@
}

std::string @CLASS_NAME@::getStateName(const @ENUM_STATES@ state)
{
    std::string stateName;

    switch(state)
    {
~~~BLOCK:ENUM_STATES_ITEM~~~
        case @ENUM_STATES@::@ENUM_STATES_ITEM@:
            stateName = "@ENUM_STATES_ITEM@";
            break;
~~~BLOCK_END~~~
        default:
            stateName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getStateName(state);
            break;
    }

    return stateName;
}

std::string @CLASS_NAME@::getEventName(const @ENUM_EVENTS@ event)
{
    std::string eventName;

    switch(event)
    {
~~~BLOCK:ENUM_EVENTS_ITEM~~~
        case @ENUM_EVENTS@::@ENUM_EVENTS_ITEM@:
            eventName = "@ENUM_EVENTS_ITEM@";
            break;
~~~BLOCK_END~~~
        default:
            eventName = hsmcpp::HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>::getEventName(event);
            break;
    }

    return eventName;
}