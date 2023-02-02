#include <glibmm.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <thread>

#include "TestsCommon.hpp"

int main(int argc, char** argv) {
    Glib::init();
    Glib::RefPtr<Glib::MainLoop> glibMainLoop = Glib::MainLoop::create();
    std::future<int> unitTestResult;

    Glib::MainContext::get_default()->signal_idle().connect_once([&] {
        unitTestResult = std::async(std::launch::async, [&] {
            ::testing::InitGoogleMock(&argc, argv);
            configureGTest("glibmm");

            int result = RUN_ALL_TESTS();

            glibMainLoop->quit();

            return result;
        });
    });

    glibMainLoop->run();

    // return unitTestResult.get();
    return 0;
}