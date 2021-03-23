#include "TestsCommon.hpp"
#include <mutex>
#include <condition_variable>
#include "ConfigurableEventListener.hpp"

#if defined(TEST_HSM_GLIB)
  #include <glib.h>
#elif defined(TEST_HSM_GLIBMM)
  #include <glibmm.h>
#endif

std::mutex gSyncCall;
std::condition_variable gMainThreadCallDoneEvent;
std::function<bool()> gFunc;
bool gCallDone;
bool gCallResult;

void configureGTest()
{
    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    auto default_printer = listeners.Release(listeners.default_result_printer());

    // add our listener, by default everything is on (the same as using the default listener)
    // here I am turning everything off so I only see the 3 lines for the result
    // (plus any failures at the end), like:

    // [==========] Running 149 tests from 53 test cases.
    // [==========] 149 tests from 53 test cases ran. (1 ms total)
    // [  PASSED  ] 149 tests.

    ConfigurableEventListener *listener = new ConfigurableEventListener(default_printer);
    listener->showEnvironment = false;
    listener->showTestCases = false;
    listener->showTestNames = false;
    listener->showSuccesses = false;
    listener->showInlineFailures = false;
    listeners.Append(listener);
}

#if defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM)
gboolean mainThreadCallback(void* data)
{
    std::lock_guard<std::mutex> lck(gSyncCall);

    gCallResult = gFunc();
    gCallDone = true;
    gMainThreadCallDoneEvent.notify_one();

    return FALSE;
}
#endif // defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM)

bool executeOnMainThread(std::function<bool()> func)
{
    // in case of STD and Qt dispatcher we can just call func()
#if defined(TEST_HSM_STD) || defined(TEST_HSM_QT)
    return func();
#endif

    std::unique_lock<std::mutex> lck(gSyncCall);

    gFunc = func;
    gCallDone = false;
    gCallResult = false;

#if defined(TEST_HSM_GLIB)
    g_idle_add(&mainThreadCallback, nullptr);
#elif defined(TEST_HSM_GLIBMM)
    const auto idle_source = Glib::IdleSource::create();

    idle_source->connect([](){ return static_cast<bool>(mainThreadCallback(nullptr)); } );
    idle_source->attach(Glib::MainContext::get_default());
#endif // TEST_HSM_GLIBMM

    gMainThreadCallDoneEvent.wait(lck, [&](){ return gCallDone; });

    return gCallResult;
}