#include "TestsCommon.hpp"

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <system_error>

#include "ConfigurableEventListener.hpp"
#include "utils/gtestbadge/BadgeEventListener.h"

#if defined(TEST_HSM_GLIB)
  #include <glib.h>
#elif defined(TEST_HSM_GLIBMM)
  #include <glibmm.h>
#elif defined(TEST_HSM_QT)
  #include <QCoreApplication>
  #include <QTimer>
#elif defined(TEST_HSM_SINGLETHREAD)
  #include <atomic>
  #include <chrono>
  #include <thread>

  #include "hsmcpp/HsmEventDispatcherSingleThread.hpp"
#endif

std::shared_ptr<hsmcpp::IHsmEventDispatcher> gDispatcher;

std::mutex gSyncCall;
std::condition_variable gMainThreadCallDoneEvent;
std::function<bool()> gFunc;
bool gCallDone;
bool gCallResult;

#if defined(TEST_HSM_SINGLETHREAD)
// The single-threaded dispatcher owns no worker thread -- the application is responsible for
// calling dispatchEvents() from its own loop. The existing suite, however, is built on blocking
// helpers (transitionSync() and the BaseAsyncHsm rendezvous) that park the test thread waiting
// for the dispatcher to run callbacks on *another* thread. To reuse that suite verbatim we give
// the dispatcher a harness-owned pump thread that plays the role of the application loop.
//
// This binary is therefore built with thread-safety ON: the test thread and the pump thread both
// touch the HSM, so the real synchronization primitives must be present. It validates the
// dispatcher's event/timer/queue logic. The no-op-sync (HSMBUILD_THREAD_SAFETY=OFF) ship
// configuration is covered separately by the inline single-threaded tests, which use no pump
// thread at all.
static std::thread gPumpThread;
static std::atomic<bool> gPumpStop{false};
static std::atomic<bool> gPumpStarted{false};

static void startSingleThreadPumpIfNeeded() {
    if ((false == gPumpStarted.load()) && gDispatcher) {
        auto st = std::static_pointer_cast<hsmcpp::HsmEventDispatcherSingleThread>(gDispatcher);
        gPumpStop = false;
        gPumpThread = std::thread([st]() {
            while (false == gPumpStop.load()) {
                st->dispatchEvents();
                // yield rather than sleep: the single-threaded dispatcher is not self-driving, so
                // the pump must sample at full resolution for repeating-timer tests to observe the
                // same number of fires a self-driving dispatcher would produce.
                std::this_thread::yield();
            }
        });
        gPumpStarted = true;
    }
}

static void stopSingleThreadPump() {
    if (true == gPumpStarted.load()) {
        gPumpStop = true;
        if (true == gPumpThread.joinable()) {
            gPumpThread.join();
        }
        gPumpStarted = false;
    }
}
#endif  // TEST_HSM_SINGLETHREAD

// Stop the pump before an HSM is torn down. For the single-threaded flavour this is called from
// RELEASE_HSM() (fixture TearDown), so the pump is never dispatching into an HSM whose destructor
// is running. The next INITIALIZE_HSM() restarts it. No-op for all other dispatchers.
void releaseSingleThreadDispatcher() {
#if defined(TEST_HSM_SINGLETHREAD)
    stopSingleThreadPump();
#endif
}

// Start the harness pump thread for the single-threaded flavour. Called from INITIALIZE_HSM()
// after the dispatcher exists, so transitions/timers enqueued during the test are dispatched.
// No-op for all other dispatchers.
void ensureSingleThreadDispatcherPump() {
#if defined(TEST_HSM_SINGLETHREAD)
    startSingleThreadPumpIfNeeded();
#endif
}

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

    bel->setTitle(name + " dispatcher tests");
    // set badge filename
    bel->setOutputFilename("tests_result_" + name + ".svg");
    // disable all console output
    bel->setSilent(true);
    // if less than 30% of test fails, show the `warning` badge type. Else, show the `failed` badge type.
    bel->setWarningRatio(0.3);
    // Google Test assumes the ownership of the listener (i.e. it will delete the listener when the test program finishes).
    listeners.Append(bel);
}

void cleanupGTest() {
    namespace fs = std::filesystem;
    std::error_code ec;

#if defined(TEST_HSM_SINGLETHREAD)
    // Stop the harness pump thread before the static dispatcher is torn down at process exit.
    stopSingleThreadPump();
#endif

    // Debug default-path case writes a fixed "./dump.hsmlog" in the working dir.
    fs::remove("./dump.hsmlog", ec);

    // Sweep any leftover TempHsmLog temp files. TempHsmLog cleans up after itself
    // in its destructor; this is a best-effort safety net for files orphaned by an
    // aborted run. Names: "hsmcpp_test_<pid>_<n>.hsmlog" under the system temp dir.
    const fs::path tempDir = fs::temp_directory_path(ec);
    if (!ec) {
        for (fs::directory_iterator it(tempDir, ec), end; (!ec) && (it != end); it.increment(ec)) {
            const std::string name = it->path().filename().string();
            if ((name.rfind("hsmcpp_test_", 0) == 0) && (it->path().extension() == ".hsmlog")) {
                std::error_code removeEc;
                fs::remove(it->path(), removeEc);
            }
        }
    }
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
#elif defined(TEST_HSM_SINGLETHREAD)
    // Run func() directly on the test thread (thread-safety is ON for this binary, so the queue
    // lock protects concurrent access with the pump thread). The pump is started/stopped
    // explicitly by INITIALIZE_HSM() / RELEASE_HSM(), not here, so RELEASE's func() can stop the
    // pump without this path immediately restarting it.
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

    idle_source->set_priority(G_PRIORITY_HIGH_IDLE);
    idle_source->connect([]() { return static_cast<bool>(mainThreadCallback(nullptr)); });
    idle_source->attach(Glib::MainContext::get_default());
  #endif  // TEST_HSM_GLIBMM

    gMainThreadCallDoneEvent.wait(lck, [&]() { return gCallDone; });

    return gCallResult;
#endif    // TEST_HSM_STD || TEST_HSM_FREERTOS
}