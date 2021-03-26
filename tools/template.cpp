// Content of this file was generated

#include "@HPP_FILE@"

@CLASS_NAME@::@CLASS_NAME@()
    : HierarchicalStateMachine<@ENUM_STATES@, @ENUM_EVENTS@>(@ENUM_STATES@::@INITIAL_STATE@)
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