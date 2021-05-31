#include "TestsCommon.hpp"
#include "hsm/ABCHsm.hpp"

// NOTE: Qt doesn't support cancelation of already posted events. So deleting dispatcher before
//       all events are processed will result in a crash.
#ifndef TEST_HSM_QT
TEST(dispatchers, DISABLED_stresstest_create_destroy)
{
    TEST_DESCRIPTION("check that it's possible to destroy HSM and disconnect from dispatcher when there are events pending");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS

    for (int i = 0; i < 10; ++i)
    {
        HierarchicalStateMachine<AbcState, AbcEvent>* hsm;

        hsm->registerState(AbcState::A, [](const VariantList_t& args){});
        hsm->registerState(AbcState::B, [](const VariantList_t& args){});
        hsm->registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
        hsm->registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

        ASSERT_TRUE(executeOnMainThread([&]()
        {
            hsm = new HierarchicalStateMachine<AbcState, AbcEvent>(AbcState::A);
            return hsm->initialize(CREATE_DISPATCHER());
        }));

        for (int j = 0 ; j < 100; ++j)
        {
            hsm->transition(AbcEvent::E1);
        }

        executeOnMainThread([&]()
        {
            delete hsm;
            return true;
        });
    }

    //-------------------------------------------
    // VALIDATION
}

#endif // TEST_HSM_QT