#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <glib.h>
#include <future>
#include <thread>
#include "TestsCommon.hpp"

GMainLoop* gMainLoop = nullptr;
std::future<int> gUnitTestResult;
int gArgc = 0;
char** gArgv = nullptr;

gboolean runTests(gpointer user_data)
{
    gUnitTestResult = std::async(std::launch::async, [&] {
        ::testing::InitGoogleMock(&gArgc, gArgv);
        configureGTest();
        
        int result = RUN_ALL_TESTS();

        g_main_loop_quit(gMainLoop);

        return result;
    });

    return FALSE;
}

int main(int argc, char** argv)
{
    gArgc = argc;
    gArgv = argv;
    gMainLoop = g_main_loop_new(nullptr, FALSE);

    g_idle_add(runTests, nullptr);

    g_main_loop_run(gMainLoop);
    g_main_loop_unref(gMainLoop);

    return gUnitTestResult.get();
}