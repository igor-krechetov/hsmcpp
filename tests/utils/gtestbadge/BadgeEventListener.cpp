#include "BadgeEventListener.h"

#include "Badge.h"
// #include "rapidassist/strings.h"
// #include "rapidassist/gtesthelp.h"

// using ra::strings::std::to_string;

const double BadgeEventListener::DEFAULT_WARNING_RATIO = 0.10;

BadgeEventListener::BadgeEventListener()
    : mSilent(false)
    , mSuccess(false)
    , mWarningRatio(DEFAULT_WARNING_RATIO) {}

BadgeEventListener::BadgeEventListener(const std::string& iFilename)
    : mSilent(false)
    , mSuccess(false)
    , mWarningRatio(DEFAULT_WARNING_RATIO)
    , mOutputFilename(iFilename) {}

BadgeEventListener::~BadgeEventListener() {}

void BadgeEventListener::OnTestProgramStart(const UnitTest& unit_test) {}

void BadgeEventListener::OnTestIterationStart(const UnitTest& unit_test, int iteration) {}

void BadgeEventListener::OnEnvironmentsSetUpStart(const UnitTest& unit_test) {}

void BadgeEventListener::OnEnvironmentsSetUpEnd(const UnitTest& unit_test) {}

void BadgeEventListener::OnTestCaseStart(const TestCase& test_case) {}

void BadgeEventListener::OnTestStart(const TestInfo& test_info) {}

void BadgeEventListener::OnTestPartResult(const TestPartResult& test_part_result) {}

void BadgeEventListener::OnTestEnd(const TestInfo& test_info) {}

void BadgeEventListener::OnTestCaseEnd(const TestCase& test_case) {}

void BadgeEventListener::OnEnvironmentsTearDownStart(const UnitTest& unit_test) {}

void BadgeEventListener::OnEnvironmentsTearDownEnd(const UnitTest& unit_test) {}

void BadgeEventListener::OnTestIterationEnd(const UnitTest& unit_test, int iteration) {}

void BadgeEventListener::OnTestProgramEnd(const UnitTest& unit_test) {
    int numDisabled = unit_test.disabled_test_count();
    int numFailed = unit_test.failed_test_count();
    int numSuccess = unit_test.successful_test_count();
    int numRun = unit_test.test_to_run_count();
    int numTotalTests = unit_test.total_test_count();

    // validate filename
    if (!mOutputFilename.empty()) {
        // detect appropriate icon
        SYSTEM_ICON icon = ICON_NONE;
        // if (ra::gtesthelp::isAppVeyor())
        // {
        //   icon = ICON_APPVEYOR;
        // }
        // else if (ra::gtesthelp::isTravis())
        // {
        //   icon = ICON_TRAVIS;
        // }

        mSuccess = generateBadge(mOutputFilename, numSuccess, numFailed, numDisabled, mTitle, icon, mWarningRatio);
        if (!mSuccess && !mSilent) {
            printf("BadgeEventListener: [ERROR] Failed saving '%s' test result badge.\n", mOutputFilename.c_str());
        }
    } else if (!mSilent) {
        printf("BadgeEventListener: [WARNING] Badge filename not specified. Skipping badge creation.\n");
    }
}

void BadgeEventListener::setTitle(const std::string& title) {
    mTitle = title;
}

void BadgeEventListener::setOutputFilename(const std::string& iFilename) {
    mOutputFilename = iFilename;
}

void BadgeEventListener::setSilent(bool iSilent) {
    mSilent = iSilent;
}

void BadgeEventListener::setWarningRatio(double iRatio) {
    mWarningRatio = iRatio;
}

bool BadgeEventListener::generateBadge(const std::string& iFilename,
                                       int success,
                                       int failures,
                                       int disabled,
                                       const std::string& title,
                                       const SYSTEM_ICON& iIcon) {
    return generateBadge(iFilename, success, failures, disabled, title, iIcon, DEFAULT_WARNING_RATIO);
}

bool BadgeEventListener::generateBadge(const std::string& iFilename,
                                       int success,
                                       int failures,
                                       int disabled,
                                       const std::string& title,
                                       const SYSTEM_ICON& iIcon,
                                       const double& iWarningRatio) {
    int total = success + failures;

    // define severity level
    const int LEVEL_SUCCESS = 0;
    const int LEVEL_WARNING = 1;
    const int LEVEL_ERROR = 2;
    int level = LEVEL_SUCCESS;
    const double failureRatio = double(failures) / double(total);
    if (failures == 0) {
        level = LEVEL_SUCCESS;
    } else if (failureRatio >= iWarningRatio) {
        level = LEVEL_ERROR;
    } else {
        level = LEVEL_WARNING;
    }

    // define badge color
    const char* color = NULL;
    switch (level) {
        case LEVEL_SUCCESS:
            color = "#4c1";  // brightgreen
            break;
        case LEVEL_WARNING:
            color = "#fe7d37";  // orange
            break;
        case LEVEL_ERROR:
        default:
            color = "#e05d44";  // red
            break;
    };

    // define right_text
    std::string right_text;
    switch (level) {
        case LEVEL_SUCCESS:
            right_text = std::to_string(success) + " passed";
            break;
        case LEVEL_WARNING:
            right_text = std::to_string(success) + " of " + std::to_string(total) + " passed";
            break;
        case LEVEL_ERROR:
        default:
            right_text = std::to_string(failures) + " of " + std::to_string(total) + " failed";
            break;
    };

    // define icon
    std::string icon = "";
    static const std::string icon_appveyor =
        "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMCAwIDQwIDQwIj4NCjxwYX"
        "RoIGZpbGw9IiNkZGQiIGQ9Ik0yMCAwYzExIDAgMjAgOSAyMCAyMHMtOSAyMC0yMCAyMFMwIDMxIDAgMjAgOSAwIDIwIDB6bTQuOSAyMy45YzIuMi0yLjgg"
        "MS45LTYuOC0uOS04LjktMi43LTIuMS02LjctMS42LTkgMS4yLTIuMiAyLjgtMS45IDYuOC45IDguOSAyLjggMi4xIDYuOCAxLjYgOS0xLjJ6bS0xMC43ID"
        "EzYzEuMi41IDMuOCAxIDUuMSAxTDI4IDI1LjNjMi44LTQuMiAyLjEtOS45LTEuOC0xMy0zLjUtMi44LTguNC0yLjctMTEuOSAwTDIuMiAyMS42Yy4zIDMu"
        "MiAxLjIgNC44IDEuMiA0LjlsNi45LTcuNWMtLjUgMy4zLjcgNi43IDMuNSA4LjggMi40IDEuOSA1LjMgMi40IDguMSAxLjhsLTcuNyA3LjN6Ii8+"
        "DQo8L3N2Zz4=";
    static const std::string icon_travis =
        "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA0MCA0MCI+PGcgZmlsbD0iI2ZmZiI+"
        "PHBhdGggZD0iTTI2LjIyMyAyMy44NDVjLjA0OCAxLjMzMy43NjEgMS4zMjMgMS42MDkgMS4yOTIgMS4xNDItLjAyMSAxLjc2OS0uMzcxIDIuMzQyLS4wOT"
        "UtLjAwOS0uMDMyLS4xODgtLjU2Ny0uOTc5LS40NzQuMDkyLS4xOTguMTMxLS40NjYuMTAzLS44MzQtLjEwNC0xLjMzMi0uODc5LTIuMzktMS43MjktMi4z"
        "Ni0uODQ4LjAyOS0xLjM5MyAxLjEzNS0xLjM0NiAyLjQ3MXptMS4yNjYtMS42MjVjLjIzOS0uMDA1LjQzOC4xODYuNDQxLjQyNi4wMDUuMjM5LS4xODcuND"
        "M4LS40MjUuNDQyLS4yNC4wMDQtLjQzOC0uMTg4LS40NDItLjQyOC0uMDA1LS4yMzkuMTg3LS40MzYuNDI2LS40NHptLTEyLjkwOCAzLjE1N2MuMDg4LjAw"
        "Mi4xOCAwIC4yNzQtLjAwMi4wODIgMCAuMTY0LS4wMDIuMjUtLjAwNS44NTEtLjAwOCAxLjU1Ni4wNzIgMS41NDUtMS4yNjItLjAxNC0xLjMzNC0uNjA2LT"
        "IuNDE0LTEuNDU3LTIuNDA1LS44NDkuMDA4LTEuNjcxIDEuMTAyLTEuNjIyIDIuNDM1LjAxNS4zNDguMDY2LjU5OC4xNDguNzc5LS43NC4xODctLjk0NS42"
        "NDQtLjk1Ny42NzMuNDY2LS4yNTEuOTkzLS4yMTUgMS44MTktLjIxM3ptLjczMS0yLjgzNWMuMjQtLjAwOC40MzkuMTgyLjQ0Ny40MjIuMDA2LjIzOS0uMT"
        "g0LjQzOS0uNDIzLjQ0NS0uMjQuMDA4LS40NC0uMTgzLS40NDYtLjQyMi0uMDA3LS4yMzkuMTgyLS40NC40MjItLjQ0NXptMS45OTctMTQuMjE1aDIuMzg2"
        "di0uOTg0aC41Mzd2NC41NDZoLS44MTF2Mi41NDNoNC4xNzd2LTIuNTQzaC0uODEyVjcuMzQzaC41Mzh2Ljk4NGgyLjM4OFY0Ljk1OGgtOC40MDMiLz48cG"
        "F0aCBkPSJNMzkuNjg5IDE0LjgyM2MtLjAxNS0uMjg5LS4wNDMtLjU3Ny0uMDgyLS44NjEtLjAyLS4xNDQtLjA0MS0uMjg4LS4wNzEtLjQyOS0uMDI4LS4x"
        "NDYtLjA1Ni0uMjgxLS4xMDMtLjQ0bC0uMDMtLjExMi0uMDc5LS4wNjNjLS40NTUtLjM1Ni0uOTQ0LS42MjYtMS40NDEtLjg2NC0uNDExLS4xOTQtLjgyOC"
        "0uMzYzLTEuMjQ5LS41MTgtMS4xNzctMy4yNjQtMy4xOTctNi4xNTgtNS44NTYtOC4xNzNDMjguMDMxIDEuMjc5IDI0LjguMTc2IDIxLjQzNi4xNzZjLTMu"
        "MzY2IDAtNi41OTYgMS4xMDMtOS4zNDQgMy4xODctMi4wOTMgMS41ODYtMy43ODggMy43MTgtNC45ODkgNi4xNDVoLS4wMDZjLS4zMjUuNjU3LS42MTcgMS"
        "4zMzMtLjg2NyAyLjAyOC0uNDIyLjE1NS0uODM5LjMyNC0xLjI1LjUxOC0uNDk4LjIzOC0uOTg2LjUwOC0xLjQ0MS44NjRsLS4wOC4wNjMtLjAzLjExMmMt"
        "LjA0Ni4xNTktLjA3Ni4yOTQtLjEwNC40NC0uMDI4LjE0MS0uMDUxLjI4NS0uMDcuNDI5LS4wMjkuMjA1LS4wNDguNDEzLS4wNjUuNjIxaC4wMDdjLS4wMD"
        "cuMDgtLjAxMy4xNi0uMDE3LjI0MS0uMDI5LjU3Ny0uMDEzIDEuMTU0LjA1IDEuNzI4LjA2MS41NzMuMTY3IDEuMTQuMzIgMS43MDMuMDguMjguMTcxLjU1"
        "NS4yNzkuODMuMDUzLjEzNy4xMTEuMjczLjE3Ni40MS4wMzIuMDY4LjA2Ny4xMzUuMTAyLjIwNGwuMDU3LjEwMWMuMDIyLjAzOC4wMzcuMDY1LjA3LjExNG"
        "wuMDU3LjA5LjA5Mi4wNDdjLjA5Ny4wNS4xNzguMDg3LjI2Ni4xMjlsLjI1Ni4xMTZjLjA0Ny4wMTkuMDkzLjAzOS4xNDIuMDU5LTEuMTY3IDEuMDc4LTIu"
        "NDM1IDIuNDc5LTMuNTMyIDQuMjExTC4xMyAyNi43NDlsMS45NDUtMS43MDdjLjAzNC0uMDI5IDEuNjY0LTEuNDQgNC4zODQtMy4wNDFsLjAzOC4zMTMuMD"
        "k5Ljc4OS40OTggNC4wNDdjLjAxMS4wNzcuMDUxLjE0Ni4xMTUuMTlsMS4wMjEuNzE3Yy4wMTguMDU3LjAzNS4xMTQuMDUyLjE3My4wMjQuMDgxLjA0Ni4x"
        "NjEuMDcxLjI0MmwuMjMzLjcwM2MuNTExIDEuNTg5IDEuMjM1IDMuMDIyIDIuMTE2IDQuMjgxLS4wNDIuMDEzLS4wODUuMDI1LS4xMjcuMDM3LS41MzEtLj"
        "I0Ni0yLjM4Ni0xLjEwNi0zLjAwOS0xLjM4N2wtMS4zMDMtLjU4NS43NzEgMS4yMDNjLjA1Ni4wODcgMS4zODYgMi4xNjEgMi40NzEgMy41NTkgMS4wMTcg"
        "MS4zMDUgMi40MDggMi42MzcgNS4wMzIgMi42MzcuMzE1IDAgLjY0Ny0uMDIuOTg3LS4wNTYuNzgzLS4wOTEgMS4zNzEtLjE2MSAxLjgxOC0uMjE5IDIuNz"
        "U0IDEuMDE0IDUuNzkyIDEuMDc0IDguNjc1LS4wMTcuNzktLjMgMS41MDktLjY2MyAyLjE2OC0xLjA3MS4wMTQtLjAwMi4wMjctLjAwNi4wMzktLjAwOSAx"
        "LjA3Mi0uMjU4IDIuMjg5LS41NDkgMi44OS0uNzE3LjA5Ni0uMDI2LjIxNy0uMDU1LjM0Mi0uMDg1LjgzNy0uMTkxIDIuMjM2LS41MTcgMi45ODEtMi4wMD"
        "kuNzUxLTEuNTA2IDEuOTc3LTMuODEyIDEuOTg5LTMuODM1TDM3IDI5LjgyOWwtMS4xMTUuNDg1Yy0uMDYxLjAyNi0xLjQ3NS42NDMtMi4yMSAxLjAyOC0u"
        "MDI1LjAxNS0uMDU3LjAyOC0uMDg2LjA0Mi41OTgtMS4yMTkuODk2LTIuMTkgMS4wMDMtMi41NjNsLjQwMy0xLjY2MyAxLjE2MS0yLjU5N3MuMzI2LTIuOD"
        "M5LjMzMy0yLjgzN2wuMTE0LTEuMDg2Yy4xMDItLjA0My43MjItLjA4Ni44NDMtLjEzNC4xNzMtLjA2OC4zNDYtLjEzNS41MTktLjIxbC4yNTUtLjExNmMu"
        "MDktLjA0Mi4xNy0uMDc5LjI2Ny0uMTI5bC4wOTItLjA0Ny4wNTgtLjA5Yy4wMzItLjA0OS4wNS0uMDc3LjA2OS0uMTE0bC4wNTYtLjEwMWMuMDM5LS4wNj"
        "kuMDcyLS4xMzYuMTAzLS4yMDQuMDY1LS4xMzYuMTIyLS4yNzIuMTc4LS40MS4xMDctLjI3NS4xOTYtLjU1LjI3Ny0uODMuMTUyLS41NjMuMjYyLTEuMTMu"
        "MzItMS43MDMuMDY0LS41NzIuMDc4LTEuMTUuMDQ5LTEuNzI3ek0xMy4xOTUgMjEuODNjLjAyNS0uMDE3IDEuMDY4LS43MDEgMi43NzEtMS41MzQgMS42OT"
        "gtLjEyOSAzLjU3MS0uMjExIDUuNTc4LS4yMTEuNDg4IDAgLjk2Ny4wMDcgMS40NC4wMTUtLjMzLjI2Ni0uNjc4LjUxNy0xLjA0LjczNGwtMS4yMDMuNzI4"
        "Yy0uNzU5LjA5Mi0xLjQxNy4yODEtMS40NTMuMjkxLS4wMjMuMDA2LS4wNDQuMDE2LS4wNjMuMDI3LS4wNTkuMDM0LS4xMDQuMDktLjEyNC4xNTZsLTEuMj"
        "MgNC4wNTEtNi41NjUgMS40NTktMi4wNjktMS40NDktLjYwMi00Ljg3M2MxLjE3NC0uMjE3IDIuNzQ2LS40NjYgNC42MzItLjY3Ny0uMjAyLjI0LS4zOTku"
        "NDk0LS41ODQuNzYybC0xLjEyMyAxLjYxMiAxLjYzNS0xLjA5MXptOC44NiAxNi45NTRjLTEuMDgzLjA3Mi0yLjE4My0uMDU1LTMuMjYtLjM1LjA5LS4wMT"
        "guMTU1LS4wMjguMjM1LS4wMzcuMDg4LS4wMTIgMi4wMjEtLjI2NiAyLjg5LTEuNTU4bC4wODIuMDAyLjIxNi0uMDAyYy4xNDMgMCAuMjczLS4wMTIuNDEy"
        "LS4wMTguMzMtLjAyNy42NDktLjA3NS45NjYtLjEzOS4wMzQuMDMzLjA2Ni4wNjYuMTAxLjEwMi42NTMuNjQ3IDEuNDY0IDEuMDE4IDIuMzU1IDEuMDgzLT"
        "EuNDk5LjYwOS0yLjkxLjg0NS0zLjk5Ny45MTd6bS4wOTgtMi4zN2MuMTYzLS4zNjMuMjg2LS42NDYuMzc5LS44NTkuMTkyLjIxNC40MzUuNDc5LjcwOC43"
        "NjItLjIxMy4wMzItLjQyMy4wNTktLjYzLjA3Ni0uMTM2LjAwNi0uMjc4LjAyMS0uNDA4LjAyMWgtLjA0OXptLTMuMjIxLTguMzYybC41ODctLjEzMmMuMD"
        "k3LS4wMjEuMTczLS4wOTQuMjAyLS4xODhsMS4yMjYtNC4wMzNjLjIxMi0uMDI4LjM5OC0uMDQzLjU1OS0uMDQzLjE1NiAwIC4zNDUuMDE1LjU1Ni4wNDNs"
        "MS4yNjIgNC4xNTNjLjAzMi4xMDYuMTIzLjE4My4yMzEuMTk1bC40NjguMDUzLjEyLjAxMyA4LjA2Mi45MTdjLjA2Ny4wMDguMTMzLS4wMTEuMTg4LS4wND"
        "lsLjgxLS41NjYuNzQ5LS41MjQuMjc3LS4xOTVjLS4wNDUuMTkzLS4wOTUuMzg1LS4xNDcuNTc0LS4wNzcuMjQ4LS4xNTkuNDg4LS4yNDEuNzI2LS4zMzEu"
        "OTQyLS43MDIgMS43OTctMS4xMDYgMi41NjktLjExLjAwNy0uMjI5LjAxMi0uMzU0LjAxMi0uMzkyIDAtLjc5LS4wMzItMS4yMzEtLjA3My0uNzY2LS4wNz"
        "EtMS45NzktLjIxLTIuMzYxLS4yNTMtLjMzMy0uMjY1LTEuNDY0LTEuMTYyLTIuNDAzLTEuOTAzLS4wMzYtLjAyOC0uMDY4LS4wNTgtLjEwMS0uMDg0LS4x"
        "NjUtLjE0LS4zNTMtLjI5OC0uNzM0LS4yOTgtLjM0OCAwLS44NTQuMTI2LTIuMDc3LjQ5NS42MzctLjQ3Ni42NDItMS4wODkuNjQyLTEuMDg5cy0uOTA2Lj"
        "g0NC0yLjM2OCAxLjAzYy0xLjQ1OC4xODgtMi42MDUtLjcwOS0yLjYwNS0uNzA5LjA0OC4xNTYuMTE2LjUwOS40NDcuODA4LS4zODktLjA0NS0uODA5LS4w"
        "NzctMS4yNS0uMDc3LS4xNTggMC0uMzIuMDA0LS40NzUuMDEyLS44OTguMDQ4LTIuMTYyIDEuNDA0LTMuMDQ5IDIuNzczLS40MzYuMTM4LTEuNzk5LjU3MS"
        "0zLjM3NSAxLjAzNC0uOTMxLTEuMjg4LTEuNTYyLTIuNTk1LTEuOTE5LTMuNDM2LS4yMDgtLjQ5LS4zMjQtLjgyMy0uMzUtLjkwMnYtLjAwMmMtLjAyOS0u"
        "MDgtLjA2LS4xNjUtLjA4Ny0uMjVsLjM3OC4yNjR2LjAwMmwuOTM1LjY1Ni4zMTIuMjE3Yy4wNDcuMDMzLjEwNC4wNS4xNTguMDUuMDIgMCAuMDQtLjAwMi"
        "4wNjEtLjAwNm0xMi45NzctNy44Yy0uMDI4LS4wOS0uMDk4LS4xNTctLjE4OC0uMTg0LS4wNDEtLjAxMy0uODUyLS4yNDQtMS43MTMtLjMxN2wuMTU4LS4w"
        "MTJjLjEzNi0uMDEgMi4zOTctLjI4NiA1LjA5OC0xLjIxOCAyLjk5MS4yMjkgNS40MzMuNTk2IDcuMTAzLjlsLS41OTEgNC4xNDYtMi4wODYgMS40NTktNi"
        "41NTctLjc0NW0xMy4xNzEtOS42MTRjLS4wNzMuNTM0LS4zMTUgMS41NzQtLjMxNSAxLjU3NGwtLjE4OS4wMzFjLS4wOTktLjAyNy0xLjAyMS0uMjY2LTIu"
        "NjA5LS41Ni4wNTEtLjAxMS4xLS4wMjIuMTQ4LS4wMzMuMzI5LS4wODIuNjUzLS4xODMuOTY4LS4zMS4xNTYtLjA2Ny4zMTItLjEzOC40NTktLjIyMi4xND"
        "YtLjA4NS4yODktLjE3OS40MDEtLjMwNi0xLjU5LjUyLTQuODc2LjI4NC03Ljc5Ny0uMDE4LTIuNjY5LS4yNzYtNS4zNDgtLjQ1NC04LjA0LS40Ni0yLjY5"
        "MS4wMDYtNS4zNjcuMTg1LTguMDM5LjQ2LTIuOTIxLjMwMi02LjIwNi41MzgtNy43OTYuMDE4LjExMi4xMjYuMjU2LjIyMS40MDMuMzA2LjE0Ni4wODQuMz"
        "AxLjE1NC40NTcuMjIyLjMxNC4xMjYuNjQuMjI4Ljk2OS4zMS4xMTYuMDI3LjIzNC4wNTMuMzUyLjA3NC0xLjQ0OS4yNzUtMi4yODguNDkzLTIuMzgyLjUx"
        "OSAwIDAtLjMzNC4wNTQtLjQwNy0uMjAyLS4xNDUtLjUxMi0uMjQyLS44Ny0uMzE1LTEuNDAzLS4wNjktLjUzMS0uMTA0LTEuMDY4LS4xLTEuNjA0LjAwMS"
        "0uMDg4LS4wMDEtLjI2NC0uMDAxLS4yNjQuMDA1LS4xOC4wMTMtLjM2LjAzLS41NC4wMS0uMTMzLjQ5OS0uNTU5LjkxOS0uODAxLjQ1NC0uMjY0LjkzMS0u"
        "NTAyIDEuNDE3LS43MjMuNDgxLS4yMjggMi4wNC0uNTU5IDIuOTgtLjcyOS45NC0uMTcyIDQuMzAyLS42MTggNC41NTQtLjcxMi4yNTEtLjA5NiAxLjcwNC"
        "0xLjIxNiAxLjk1OC0xLjMwOS0uNTM0LjA4OC03LjkxMiAxLjIxNS04LjI0NCAxLjMyIDIuMzk1LTYuMjYzIDcuMDk2LTkuMzEzIDEzLjM1My05LjMxMyA2"
        "LjI1NSAwIDEwLjk1NyAyLjg4IDEzLjM1NCA5LjE0NC0zLjE2NC0uODQxLTcuNTQ2LTEuMjMzLTguMDgtMS4zMjIuMjU0LjA5MyAxLjY5MSAxLjEwNCAxLj"
        "k1OCAxLjEzOCAyLjE4OC4yODQgNS42MzggMS4wNDkgNS44ODggMS4xNDguNDk2LjIwMS45OTMuNDA4IDEuNDc1LjYzNS40ODcuMjIxLjk2My40NTkgMS40"
        "MTcuNzIzLjQyLjI0My45MDguNjY4LjkyLjgwMS4wMjMuMjY4LjAzMy41MzUuMDM2LjgwMy4wMDQuNTM2LS4wMzMgMS4wNzQtLjEwMiAxLjYwNXoiLz48L2"
        "c+PC9zdmc+";
    switch (iIcon) {
        case ICON_APPVEYOR:
            icon = icon_appveyor;
            break;
        case ICON_TRAVIS:
            icon = icon_travis;
            break;
    };

    Badge b;
    b.setBase64Icon(icon);
    b.setRightBackgroundColor(color);
    b.setLeftText(title);
    b.setRightText(right_text);
    bool saved = b.save(iFilename.c_str());

    return saved;
}
