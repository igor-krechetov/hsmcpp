#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <thread>
#include "TestsCommon.hpp"
#include <QCoreApplication>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    std::future<int> unitTestResult;

    unitTestResult = std::async(std::launch::async, [&] {
        ::testing::InitGoogleMock(&argc, argv);
        configureGTest();

        int result = RUN_ALL_TESTS();

        QCoreApplication::exit(result);

        return result;
    });

    return app.exec();
}