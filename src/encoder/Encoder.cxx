#include "FreeRTOS.h"
#include "task.h"

#include "Encoder.hpp"
#include "gcem.hpp"
#include "helpers/freertos.hpp"
#include "tim.h"
#include "units/si/frequency.hpp"
#include <algorithm>

void Encoder::taskMain(void *)
{
    HAL_TIM_Encoder_Start(encoderTimer, TIM_CHANNEL_ALL);

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));

        // get new encoder value and calc difference
        int diff = calculateDiff();

        if (diff == 0)
            continue;

        ledFading.resetLedIdleTimeout();

        int16_t newTarget = ledFading.getTargetPercentage();
        constexpr auto PercentageChangeStep = 5;
        newTarget += diff * PercentageChangeStep;

        newTarget = std::clamp<int16_t>(
            newTarget, LedFading::MinPercentage,
            isOverTemperature
                ? LedFading::DefaultPercentage // in case of over temp, set to max. 80%
                : LedFading::MaxPercentage);

        // set target LED percentage and start fading
        ledFading.setTargetPercentage(newTarget);
        ledFading.setFadingState(LedFading::FadingState::Normal);
        ledFading.notify(1U, util::wrappers::NotifyAction::SetBits);
    }
}

// ----------------------------------------------------------------------------
int Encoder::calculateDiff()
{
    static uint16_t prevEncoderValue = 0;
    const uint16_t EncoderValue = __HAL_TIM_GET_COUNTER(encoderTimer);
    int diff = (EncoderValue - prevEncoderValue);

    if (diff == 0 || gcem::abs(diff) < 4)
        return 0;

    if (diff >= std::numeric_limits<uint16_t>::max() / 2)
        diff = std::numeric_limits<uint16_t>::max() - diff;

    else if (diff <= -std::numeric_limits<uint16_t>::max() / 2)
        diff = std::numeric_limits<uint16_t>::max() + diff;

    // STM encoder timer returns values by factor 4 however
    diff = (diff / 4);
    prevEncoderValue = EncoderValue;

    return diff;
}