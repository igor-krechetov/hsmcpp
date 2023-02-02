// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include <FreeRTOS.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <task.h>

#include "TestsCommon.hpp"

int gArgc = 0;
char** gArgv = nullptr;

extern "C" {
void startTests(void* pvParameters) {
    ::testing::InitGoogleMock(&gArgc, gArgv);
    configureGTest();

    static_cast<void>(RUN_ALL_TESTS());
    vTaskEndScheduler();
}
}

int main(int argc, char** argv) {
    gArgc = argc;
    gArgv = argv;

    xTaskCreate(startTests, "startTests", configMINIMAL_STACK_SIZE, nullptr, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();

    return 0;
}

// FreeRTOS callbacks
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

extern "C" {
void vAssertCalled(const char* const pcFileName, unsigned long ulLine) {}

void vApplicationMallocFailedHook(void) {
    vAssertCalled(__FILE__, __LINE__);
}

void vApplicationTickHook(void) {}

void vApplicationDaemonTaskStartupHook(void) {}

void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
// /*-----------------------------------------------------------*/

void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
}