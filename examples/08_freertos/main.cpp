// Copyright (C) 2022 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include <stdio.h>

// FreeRTOS kernel includes
#include <FreeRTOS.h>
#include <task.h>

// hsmcpp
#include <hsmcpp/HsmEventDispatcherFreeRTOS.hpp>
#include "SwitchHsm.hpp"


extern "C"
{
    void taskInitHSM(void* pvParameters);
    void taskTransitions(void* pvParameters);
}

SwitchHsm* hsm = nullptr;

int main()
{
    printf("BEGIN\n");
    
    xTaskCreate(taskInitHSM, "taskInitHSM", configMINIMAL_STACK_SIZE, nullptr, tskIDLE_PRIORITY, NULL);

    // Start the tasks and timer running.
    printf("vTaskStartScheduler\n");
    vTaskStartScheduler();
    printf("vTaskStartScheduler - DONE\n");

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for( ; ; )
    {
    }
}

extern "C"
{
    void taskInitHSM(void* pvParameters)
    {
        printf("taskInitHSM\n");
        // NOTE: it's important to set correct priority for dispatcher
        std::shared_ptr<hsmcpp::HsmEventDispatcherFreeRTOS> dispatcher = std::make_shared<hsmcpp::HsmEventDispatcherFreeRTOS>();

        hsm = new SwitchHsm();
        // NOTE: initialize() must be called from a Task!
        hsm->initialize(dispatcher);

        xTaskCreate(taskTransitions, "taskTransitions", configMINIMAL_STACK_SIZE, hsm, tskIDLE_PRIORITY, NULL);
        vTaskSuspend(nullptr);

        // we should never reach this part, but adding resources cleanup just in case
        hsm->release();
        delete hsm;
        hsm = nullptr;
        vTaskDelete(nullptr);
    }

    void taskTransitions(void* pvParameters)
    {
        SwitchHsm* hsm = static_cast<SwitchHsm*>(pvParameters);
        printf("taskTransitions - BEGIN\n");

        while(true)
        {
            vTaskDelay(800 / portTICK_PERIOD_MS);

            if (true == hsm->isInitialized())
            {
                printf("SwitchHsmEvents::SWITCH --> HSM\n");
                hsm->transition(SwitchHsmEvents::SWITCH);
            }
        }

        vTaskDelete(nullptr);
    }
}

