#include "hsm/AsyncHsm.hpp"

TEST_F(AsyncHsm, multithreaded_entrypoint_cancelation)
{
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be cancled");

    //-------------------------------------------
    // PRECONDITIONS
    registerState(AsyncHsmState::A, this, &AsyncHsm::onStateChanged, nullptr, &AsyncHsm::onExit);
    registerState(AsyncHsmState::B, this, &AsyncHsm::onStateChanged, nullptr, nullptr);
    registerState(AsyncHsmState::C, this, &AsyncHsm::onStateChanged, nullptr, nullptr);

    ASSERT_TRUE(registerSubstate(AsyncHsmState::P1, AsyncHsmState::B, true));

    registerTransition(AsyncHsmState::A, AsyncHsmState::P1, AsyncHsmEvent::NEXT_STATE, nullptr, nullptr);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::C, AsyncHsmEvent::EXIT_SUBSTATE, nullptr, nullptr);
    registerTransition(AsyncHsmState::C, AsyncHsmState::A, AsyncHsmEvent::NEXT_STATE, nullptr, nullptr);

    ASSERT_EQ(getCurrentState(), AsyncHsmState::A);

    //-------------------------------------------
    // ACTIONS
    // this should trigger A -> [P1 -> B]
    ASSERT_TRUE(transitionEx(AsyncHsmEvent::NEXT_STATE, false, false));
    waitAsyncOperation(200);// wait for A::onExit

    // send new event with clearQueue=TRUE
    ASSERT_TRUE(transitionEx(AsyncHsmEvent::EXIT_SUBSTATE, true, false));

    unblockNextStep();// allow A::onExit to continue
    waitAsyncOperation(200);// wait for B::onStateChanged

    // NOTE: this is the main validation point. In case of an error state would be C since we would never go into B
    ASSERT_EQ(getCurrentState(), AsyncHsmState::B);
    unblockNextStep();// allow B::onStateChanged to continue

    waitAsyncOperation(200);// wait for C::onStateChanged
    ASSERT_EQ(getCurrentState(), AsyncHsmState::C);
    unblockNextStep();// allow C::onStateChanged to continue
    usleep(200 * 1000);// ugly, but we need to let HSM to return controll to main loop
    // otherwise gTest will free resources while we are still inside HSM handler

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), AsyncHsmState::C);

}