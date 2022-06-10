#include "FreeRTOS.h"
#include "task.h"

#include "gcem/include/gcem.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"
#include "tim.h"
#include "units/si/frequency.hpp"

extern TaskHandle_t fadingHandle;
extern FadingState fadingState;
extern bool isOverTemperature;
extern void resetLedIdleTimeout();

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

        // get new encoder value and calc difference
        const uint16_t EncoderValue = __HAL_TIM_GET_COUNTER(EncoderTimer);
        int8_t diff = (EncoderValue - oldEncoderValue);

        if (diff == 0 || gcem::abs(diff) < 4)
            continue;

        resetLedIdleTimeout();

        // STM encoder timer returns values by factor 4 however
        diff = (diff / 4) * 5;
        oldEncoderValue = EncoderValue;

        int16_t temp = targetLedPercentage;
        temp += diff;

        if (temp < MinPercentage)
            temp = MinPercentage;

        else if (!isOverTemperature)
        {
            if (temp > MaxPercentage)
                temp = MaxPercentage;
        }
        else
        {
            // in case of over temperature default is our new maximum
            if (temp > DefaultPercentage)
                temp = DefaultPercentage;
        }

        // set target LED percentage and start fading
        targetLedPercentage = temp;
        fadingState = FadingState::Normal;
        xTaskNotify(fadingHandle, 1U, eSetBits);
    }
}
