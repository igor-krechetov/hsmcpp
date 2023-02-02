#include "TestsCommon.hpp"

#include <condition_variable>
#include <mutex>

#include "ConfigurableEventListener.hpp"
#include "utils/gtestbadge/BadgeEventListener.h"

#if defined(TEST_HSM_GLIB)
  #include <glib.h>
#elif defined(TEST_HSM_GLIBMM)
  #include <glibmm.h>
#elif defined(TEST_HSM_QT)
  #include <QCoreApplication>
  #include <QTimer>
#endif

std::mutex gSyncCall;
std::condition_variable gMainThreadCallDoneEvent;
std::function<bool()> gFunc;
bool gCallDone;
bool gCallResult;

void configureGTest(const std::string& name) {
    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();

#ifdef LOGGING_MODE_OFF
    auto default_printer = listeners.Release(listeners.default_result_printer());

    // add our listener, by default everything is on (the same as using the default listener)
    // here I am turning everything off so I only see the 3 lines for the result
    // (plus any failures at the end), like:

    // [==========] Running 149 tests from 53 test cases.
    // [==========] 149 tests from 53 test cases ran. (1 ms total)
    // [  PASSED  ] 149 tests.

    ConfigurableEventListener* listener = new ConfigurableEventListener(default_printer);

    listener->showEnvironment = false;
    listener->showTestCases = false;
    listener->showTestNames = false;
    listener->showSuccesses = false;
    listener->showInlineFailures = false;
    listeners.Append(listener);
#endif  // LOGGING_MODE_OFF

    // Create and register a new BadgeEventListener
    BadgeEventListener* bel = new BadgeEventListener();

    bel->setOutputFilename("tests_result_" + name + ".svg");  // set badge filename
    bel->setSilent(true);                                     // disable all console output
    bel->setWarningRatio(
        0.3);  // if less than 30% of test fails, show the `warning` badge type. Else, show the `failed` badge type.
    listeners.Append(bel);  // Google Test assumes the ownership of the listener (i.e. it will delete the listener when the test
                            // program finishes).
}

#if defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM) || defined(TEST_HSM_QT)
  #if defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM)
gboolean mainThreadCallback(void* data)
  #elif defined(TEST_HSM_QT)
void mainThreadCallback()
  #endif
{
    std::lock_guard<std::mutex> lck(gSyncCall);

    gCallResult = gFunc();
    gCallDone = true;
    gMainThreadCallDoneEvent.notify_one();

  #if defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM)
    return FALSE;
  #endif
}
#endif  // defined(TEST_HSM_GLIB) || defined(TEST_HSM_GLIBMM)

bool executeOnMainThread(std::function<bool()> func) {
    // in case of some dispatchers we can just call func()
#if defined(TEST_HSM_STD) || defined(TEST_HSM_FREERTOS)
    return func();
#else
    std::unique_lock<std::mutex> lck(gSyncCall);

    gFunc = func;
    gCallDone = false;
    gCallResult = false;

  #if defined(TEST_HSM_QT)
    QTimer* timer = new QTimer();

    timer->moveToThread(qApp->thread());
    timer->setSingleShot(true);

    QObject::connect(timer, &QTimer::timeout, [=]() {
        mainThreadCallback();
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
  #elif defined(TEST_HSM_GLIB)
    g_idle_add(&mainThreadCallback, nullptr);
  #elif defined(TEST_HSM_GLIBMM)
    const auto idle_source = Glib::IdleSource::create();

    idle_source->connect([]() { return static_cast<bool>(mainThreadCallback(nullptr)); });
    idle_source->attach(Glib::MainContext::get_default());
  #endif  // TEST_HSM_GLIBMM

    gMainThreadCallDoneEvent.wait(lck, [&]() { return gCallDone; });

    return gCallResult;
#endif    // TEST_HSM_STD || TEST_HSM_FREERTOS
}