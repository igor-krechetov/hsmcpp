#ifndef HSMCPP_TESTS_CONFIGURABLEEVENTLISTENER_HPP
#define HSMCPP_TESTS_CONFIGURABLEEVENTLISTENER_HPP

#include <gtest/gtest.h>

// based on: https://gist.github.com/elliotchance/8215283
class ConfigurableEventListener : public testing::TestEventListener {
public:
    // Show the names of each test case.
    bool showTestCases;
    // Show the names of each test.
    bool showTestNames;
    // Show each success.
    bool showSuccesses;
    // Show each failure as it occurs. You will also see it at the bottom after the full suite is run.
    bool showInlineFailures;
    // Show the setup of the global environment.
    bool showEnvironment;

    explicit ConfigurableEventListener(TestEventListener* theEventListener)
        : eventListener(theEventListener) {
        showTestCases = true;
        showTestNames = true;
        showSuccesses = true;
        showInlineFailures = true;
        showEnvironment = true;
    }

    virtual ~ConfigurableEventListener() {
        delete eventListener;
    }

    virtual void OnTestProgramStart(const testing::UnitTest& unit_test) {
        eventListener->OnTestProgramStart(unit_test);
    }

    virtual void OnTestIterationStart(const testing::UnitTest& unit_test, int iteration) {
        eventListener->OnTestIterationStart(unit_test, iteration);
    }

    virtual void OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test) {
        if (showEnvironment) {
            eventListener->OnEnvironmentsSetUpStart(unit_test);
        }
    }

    virtual void OnEnvironmentsSetUpEnd(const testing::UnitTest& unit_test) {
        if (showEnvironment) {
            eventListener->OnEnvironmentsSetUpEnd(unit_test);
        }
    }

    virtual void OnTestCaseStart(const testing::TestCase& test_case) {
        if (showTestCases) {
            eventListener->OnTestCaseStart(test_case);
        }
    }

    virtual void OnTestStart(const testing::TestInfo& test_info) {
        if (showTestNames) {
            eventListener->OnTestStart(test_info);
        }
    }

    virtual void OnTestPartResult(const testing::TestPartResult& result) {
        eventListener->OnTestPartResult(result);
    }

    virtual void OnTestEnd(const testing::TestInfo& test_info) {
        if ((showInlineFailures && test_info.result()->Failed()) || (showSuccesses && !test_info.result()->Failed())) {
            eventListener->OnTestEnd(test_info);
        }
    }

    virtual void OnTestCaseEnd(const testing::TestCase& test_case) {
        if (showTestCases) {
            eventListener->OnTestCaseEnd(test_case);
        }
    }

    virtual void OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test) {
        if (showEnvironment) {
            eventListener->OnEnvironmentsTearDownStart(unit_test);
        }
    }

    virtual void OnEnvironmentsTearDownEnd(const testing::UnitTest& unit_test) {
        if (showEnvironment) {
            eventListener->OnEnvironmentsTearDownEnd(unit_test);
        }
    }

    virtual void OnTestIterationEnd(const testing::UnitTest& unit_test, int iteration) {
        eventListener->OnTestIterationEnd(unit_test, iteration);
    }

    virtual void OnTestProgramEnd(const testing::UnitTest& unit_test) {
        eventListener->OnTestProgramEnd(unit_test);
    }

protected:
    TestEventListener* eventListener;
};

#endif  // HSMCPP_TESTS_CONFIGURABLEEVENTLISTENER_HPP