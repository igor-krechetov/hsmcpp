#include <gtest/gtest.h>
#include <stdio.h>

#include <iostream>
#include <string>

using namespace testing;

class BadgeEventListener : public ::testing::TestEventListener {
public:
    BadgeEventListener();
    BadgeEventListener(const std::string& iFilename);
    virtual ~BadgeEventListener();

    // TestEventListener methods
    virtual void OnTestProgramStart(const UnitTest& unit_test);
    virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
    virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
    virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test);
    virtual void OnTestCaseStart(const TestCase& test_case);
    virtual void OnTestStart(const TestInfo& test_info);
    virtual void OnTestPartResult(const TestPartResult& test_part_result);
    virtual void OnTestEnd(const TestInfo& test_info);
    virtual void OnTestCaseEnd(const TestCase& test_case);
    virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
    virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test);
    virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);
    virtual void OnTestProgramEnd(const UnitTest& unit_test);

    void setTitle(const std::string& title);

    ///< summary>Set the output file path used for saving.</summary>
    ///< param name="iFilename">The output file path used for saving.</param>
    void setOutputFilename(const std::string& iFilename);

    ///< summary>Get the output file path used for saving.</summary>
    ///< returns>Returns the output file path used for saving.</returns>
    const std::string& getOutputFilename() const {
        return mOutputFilename;
    }

    ///< summary>Sets the 'silent' flag.</summary>
    ///< param name="iSilent">True to disable warnings/errors messages on the console.</param>
    void setSilent(bool iSilent);

    ///< summary>Provides the 'silent' flag.</summary>
    ///< returns>Returns true if warnings/errors messages on the console are disable. Returns false otherwise.</returns>
    bool isSilent() const {
        return mSilent;
    }

    ///< summary>Returns the badge creation success flag.</summary>
    ///< returns>Returns true if the badge was created. Returns false otherwise.</returns>
    bool getSuccess() const {
        return mSuccess;
    }

    ///< summary>
    /// Set the warning ratio.
    /// The warning ratio defines the number of test failures at which the badge shows a red background instead of an orange
    /// background. The ratio must be between 0 and 1. For instance:
    ///   Set the ratio to 0.1 to show a red background if 10% or more of test fails.
    ///   Set the ratio to 0.3 to show the `warning` badge if less than 30% of test fails. Else, show the `failed` badge type.
    ///   Set to 0.0 to disable the 'warning' badge functionality.
    ///</summary>
    ///< param name="ratio">The warning ratio from 0 to 1.</param>
    void setWarningRatio(double iRatio);

    ///< summary>Get the warning ratio.</summary>
    ///< returns>Returns the warning ratio.</returns>
    const double& getWarningRatio() const {
        return mWarningRatio;
    }

    ///< summary>Default value for warning ratio. See <see cref="setWarningRatio(double)"/> for details. </summary>
    static const double DEFAULT_WARNING_RATIO;

    ///< summary>Define the list of supported icon.</summary>
    enum SYSTEM_ICON {
        ICON_NONE,
        ICON_APPVEYOR,
        ICON_TRAVIS,
    };

    ///< summary>Generate a test badge based on the given test results.</summary>
    ///< param name="iFilename">The file path to save the image.</param>
    ///< param name="success">The number of successful tests.</param>
    ///< param name="failures">The number of failed tests.</param>
    ///< param name="disabled">The number of disabled tests.</param>
    ///< param name="iIcon">The id of a badge icon.</param>
    ///< returns>Returns true if the badge was properly saved. Returns false otherwise.</returns>
    static bool generateBadge(const std::string& iFilename, int success, int failures, int disabled, const std::string& title, const SYSTEM_ICON& iIcon);

    ///< summary>Generate a test badge based on the given test results.</summary>
    ///< param name="iFilename">The file path to save the image.</param>
    ///< param name="success">The number of successful tests.</param>
    ///< param name="failures">The number of failed tests.</param>
    ///< param name="disabled">The number of disabled tests.</param>
    ///< param name="iIcon">The id of a badge icon.</param>
    ///< param name="iWarningRatio">The warning ratio to use. See <see cref="setWarningRatio(double)"/> for details.</param>
    ///< returns>Returns true if the badge was properly saved. Returns false otherwise.</returns>
    static bool generateBadge(const std::string& iFilename,
                              int success,
                              int failures,
                              int disabled,
                              const std::string& title,
                              const SYSTEM_ICON& iIcon,
                              const double& iWarningRatio);

private:
    std::string mOutputFilename;
    std::string mTitle = "tests";
    double mWarningRatio;
    bool mSilent;
    bool mSuccess;
};
