#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <future>
#include <thread>
#include <chrono>

#include "TestsCommon.hpp"

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    std::future<int> unitTestResult;

    std::thread t1([&]() {
        ::testing::InitGoogleMock(&argc, argv);
        configureGTest("qt");

        int result = RUN_ALL_TESTS();

        // Separate thread is needed for QObject::deleteLater() to work correctly:
        // From Qt documentation:
        // Since Qt 4.8, if deleteLater() is called on an object that lives in a thread with no running event loop, the object
        // will be destroyed when the thread finishes.
        std::thread t2([result, &app] {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            QCoreApplication::exit(result);
        });

        t2.detach();
    });

    t1.detach();
    app.exec();

    return 0;
}