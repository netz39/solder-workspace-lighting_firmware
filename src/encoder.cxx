#include "FreeRTOS.h"
#include "task.h"

#include "gcem/include/gcem.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"
#include "tim.h"
#include "units/si/frequency.hpp"

extern TaskHandle_t fadingHandle;

constexpr auto EncoderTimer = &htim1;
constexpr auto TaskFrequency = 50.0_Hz;

extern "C" void encoderTask(void *)
{
    HAL_TIM_Encoder_Start(EncoderTimer, TIM_CHANNEL_ALL);
    uint16_t oldEncoderValue = 0;

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));

        const uint16_t EncoderValue = TIM1->CNT;
        int8_t diff = (EncoderValue - oldEncoderValue);

        if (diff == 0 || gcem::abs(diff) < 4)
            continue;

        diff = (diff / 4) * 5;
        oldEncoderValue = EncoderValue;

        int16_t temp = targetLedPercentage;
        temp += diff;

        if (temp < MinPercentage)
            temp = MinPercentage;

        else if (temp > MaxPercentage)
            temp = MaxPercentage;

        targetLedPercentage = temp;
        xTaskNotify(fadingHandle, 1U, eSetBits);
    }
}
