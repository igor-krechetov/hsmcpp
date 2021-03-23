#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <glibmm.h>
#include <future>
#include <thread>
#include "hsmcpp/logging.hpp"
#include "TestsCommon.hpp"

__TRACE_PREINIT__();

int main(int argc, char** argv)
{
    __TRACE_INIT__();

    Glib::init();
    Glib::RefPtr<Glib::MainLoop> glibMainLoop = Glib::MainLoop::create();
    std::future<int> unitTestResult;

    Glib::MainContext::get_default()->signal_idle().connect_once([&] {
        unitTestResult = std::async(std::launch::async, [&] {
            ::testing::InitGoogleMock(&argc, argv);
            configureGTest();

            int result = RUN_ALL_TESTS();

            glibMainLoop->quit();

            return result;
        });
    });

    glibMainLoop->run();

    return unitTestResult.get();
}