#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "timers.h"

#include "GammaLUT.hpp"
#include "led_control/LedFading.hpp"
#include "helpers/freertos.hpp"
#include "units/si/frequency.hpp"

#include <climits>

void LedFading::onLedIdleTimeout()
{
    fadingState = FadingState::Standby;
    targetLedPercentage = MinPercentage;
    notify(1U, util::wrappers::NotifyAction::SetBits);
}

void LedFading::resetLedIdleTimeout()
{
    xTimerReset(ledIdleTimer, 0);
}

void LedFading::taskMain()
{
    HAL_TIM_PWM_Start(ledTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(ledTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(ledTimer, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(ledTimer, TIM_CHANNEL_4);

    resetLedIdleTimeout();

    auto lastWakeTime = xTaskGetTickCount();
    bool restart = true;

    while (true)
    {
        if (!restart)
            notifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;

        const int8_t Difference = currentLedPercentage - targetLedPercentage;
        const uint8_t NumberOfSteps = gcem::abs(Difference);

        units::si::Time delayTime = 0.0_s;
        switch (fadingState)
        {
        case FadingState::Normal:
            delayTime = TimeToFade;
            break;

        case FadingState::Standby:
            delayTime = TimeToFadeOff;
            break;
        }
        delayTime = delayTime / NumberOfSteps;

        uint8_t factor = NumberOfSteps - 1;

        while (true)
        {
            currentLedPercentage = targetLedPercentage + (factor * Difference) / NumberOfSteps;

            const auto PwmValue = GammaLUT[currentLedPercentage];
            ledTimer->Instance->CCR1 = PwmValue;
            ledTimer->Instance->CCR2 = PwmValue;
            ledTimer->Instance->CCR3 = PwmValue;
            ledTimer->Instance->CCR4 = PwmValue;

            if (factor == 0)
                break;

            factor--;

            uint32_t notifiedValue;
            notifyWait(0, ULONG_MAX, &notifiedValue, toOsTicks(delayTime));
            if ((notifiedValue & 1U) != 0)
            {
                // restart fading
                restart = true;
                break;
            }
        }
    }
}