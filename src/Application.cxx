#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "Application.hpp"
#include "core/SafeAssert.h"
#include "wrappers/Task.hpp"

#include <memory>

extern "C" void StartDefaultTask(void *) // NOLINT
{
    static auto app = std::make_unique<Application>();
    app->run();

    SafeAssert(false); // this line should be never reached
}

//--------------------------------------------------------------------------------------------------
Application::Application()
{
    SafeAssert(instance == nullptr);
    instance = this;
    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_ADC_RegisterCallback(
        AdcPeripherie, HAL_ADC_CONVERSION_COMPLETE_CB_ID,
        [](ADC_HandleTypeDef *) //
        { getApplicationInstance().analogToDigital.conversionCompleteCallback(); });

    result = HAL_SPI_RegisterCallback(
        DisplaySpiPeripherie, HAL_SPI_TX_COMPLETE_CB_ID,
        [](SPI_HandleTypeDef *) //
        { getApplicationInstance().oledDisplay.notifyRenderTaskFromISR(); });

    SafeAssert(result == HAL_OK);
}

//--------------------------------------------------------------------------------------------------
[[noreturn]] void Application::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
Application &Application::getApplicationInstance()
{
    // Not constructing Application in this singleton, to avoid bugs where something tries to
    // access this function, while application constructs which will cause infinite recursion
    return *instance;
}

//--------------------------------------------------------------------------------------------------
void Application::timeoutCallback(TimerHandle_t timer)
{
    getApplicationInstance().ledFading.onLedIdleTimeout();
}