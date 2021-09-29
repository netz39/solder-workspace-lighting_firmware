#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"

#include "GammaLUT.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"
#include "units/si/frequency.hpp"

#include <climits>

uint8_t targetLedPercentage = MaxPercentage * 0.8;

namespace
{
constexpr auto LedTimer = &htim2;
constexpr auto TaskFrequency = 100.0_Hz;
uint8_t currentLedPercentage = MinPercentage;
} // namespace

extern "C" void fadingTask(void *)
{
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_4);

    constexpr auto TimeToFadeAtStart = 300.0_ms;
    constexpr auto TimeToFade = 100.0_ms;

    bool isStarting = true;

    auto lastWakeTime = xTaskGetTickCount();
    bool restart = true;

    while (true)
    {
        if (!restart)
            xTaskNotifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;

        const int8_t Difference = currentLedPercentage - targetLedPercentage;
        const uint8_t NumberOfSteps = gcem::abs(Difference);

        const auto DelayTime = (isStarting ? TimeToFadeAtStart : TimeToFade) / NumberOfSteps;

        uint8_t factor = NumberOfSteps - 1;

        while (true)
        {
            currentLedPercentage = targetLedPercentage + (factor * Difference) / NumberOfSteps;

            const auto PwmValue = GammaLUT[currentLedPercentage];
            LedTimer->Instance->CCR1 = PwmValue;
            LedTimer->Instance->CCR2 = PwmValue;
            LedTimer->Instance->CCR3 = PwmValue;
            LedTimer->Instance->CCR4 = PwmValue;

            if (factor == 0)
                break;

            factor--;

            uint32_t notifiedValue;
            xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, toOsTicks(DelayTime));
            if ((notifiedValue & 1U) != 0)
            {
                // restart fading

                restart = true;
                break;
            }
        }
        isStarting = false;
    }
}