#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <glibmm.h>
#include <future>
#include <thread>
#include "logging.hpp"

__TRACE_PREINIT__();

int main(int argc, char** argv)
{
    __TRACE_INIT__();

    Glib::init();
    Glib::RefPtr<Glib::MainLoop> glibMainLoop = Glib::MainLoop::create();
    std::future<int> unitTestResult;

    // NOTE: Current tests structure has an issue due to HSM being created in a different thread from Glib Main Loop.
    //       Once in awhile a following error might appear (though it's quite rare, about once per 100 repeat circles):
    //
    //   glibmm-CRITICAL **: 14:29:01.091: {anonymous}::SourceCallbackData* {anonymous}::glibmm_source_get_callback_data(GSource*): assertion 'callback_funcs != nullptr' failed

    // TODO: for glib based dispatchers move HSM creation to glib thread.

    Glib::MainContext::get_default()->signal_idle().connect_once([&] {
        unitTestResult = std::async(std::launch::async, [&] {
            ::testing::InitGoogleMock(&argc, argv);
            int result = RUN_ALL_TESTS();

            glibMainLoop->quit();

            return result;
        });
    });

    glibMainLoop->run();

    return unitTestResult.get();
}