#include "FreeRTOS.h"
#include "task.h"

#include "Encoder.hpp"
#include "gcem.hpp"
#include "helpers/freertos.hpp"
#include "tim.h"
#include "units/si/frequency.hpp"

void Encoder::taskMain()
{
    HAL_TIM_Encoder_Start(encoderTimer, TIM_CHANNEL_ALL);
    uint16_t oldEncoderValue = 0;

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));

        // get new encoder value and calc difference
        const uint16_t EncoderValue = __HAL_TIM_GET_COUNTER(encoderTimer);
        int diff = (EncoderValue - oldEncoderValue);

        if (diff == 0 || gcem::abs(diff) < 4)
            continue;

        if (diff >= std::numeric_limits<uint16_t>::max() / 2)
            diff = std::numeric_limits<uint16_t>::max() - diff;

        else if (diff <= -std::numeric_limits<uint16_t>::max() / 2)
            diff = std::numeric_limits<uint16_t>::max() + diff;

        ledFading.resetLedIdleTimeout();

        // STM encoder timer returns values by factor 4 however
        diff = (diff / 4) * 5;
        oldEncoderValue = EncoderValue;

        int16_t temp = ledFading.getTargetPercentage();
        temp += diff;

        if (temp < LedFading::MinPercentage)
            temp = LedFading::MinPercentage;

        else if (!isOverTemperature)
        {
            if (temp > LedFading::MaxPercentage)
                temp = LedFading::MaxPercentage;
        }
        else
        {
            // in case of over temperature default is our new maximum
            if (temp > LedFading::DefaultPercentage)
                temp = LedFading::DefaultPercentage;
        }

        // set target LED percentage and start fading
        ledFading.setTargetPercentage(temp);
        ledFading.setFadingState(LedFading::FadingState::Normal);
        ledFading.notify(1U, util::wrappers::NotifyAction::SetBits);
    }
}