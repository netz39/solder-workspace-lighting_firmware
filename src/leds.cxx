#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "timers.h"

#include "GammaLUT.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"
#include "units/si/frequency.hpp"

#include <climits>

extern TaskHandle_t fadingHandle;
uint8_t targetLedPercentage = DefaultPercentage;

enum class FadingState
{
    Starting,
    Normal,
    Standby
};

FadingState fadingState = FadingState::Starting;

namespace
{
constexpr auto LedTimer = &htim2;
constexpr auto TaskFrequency = 100.0_Hz;

constexpr auto TimeToFadeAtStart = 300.0_ms;
constexpr auto TimeToFade = 100.0_ms;
constexpr auto TimeToFadeOff = 5.0_s;
constexpr auto LedIdleTimout = 45.0_min;

uint8_t currentLedPercentage = MinPercentage;

//--------------------------------------------------------------------------------------------------
void onLedIdleTimeout(TimerHandle_t)
{
    fadingState = FadingState::Standby;
    targetLedPercentage = MinPercentage;
    xTaskNotify(fadingHandle, 1U, eSetBits);
}

TimerHandle_t ledIdleTimer =
    xTimerCreate("ledIdleTimeout", toOsTicks(LedIdleTimout), pdFALSE, nullptr, onLedIdleTimeout);

} // namespace

void resetLedIdleTimeout()
{
    xTimerGetExpiryTime xTimerReset(ledIdleTimer, 0);
    fadingState = FadingState::Normal;
}

extern "C" void fadingTask(void *)
{
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_4);

    auto lastWakeTime = xTaskGetTickCount();
    bool restart = true;

    while (true)
    {
        if (!restart)
            xTaskNotifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;

        const int8_t Difference = currentLedPercentage - targetLedPercentage;
        const uint8_t NumberOfSteps = gcem::abs(Difference);

        units::si::Time delayTime = 0.0_s;
        switch (fadingState)
        {
        case FadingState::Starting:
            delayTime = TimeToFadeAtStart;
            break;

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
            LedTimer->Instance->CCR1 = PwmValue;
            LedTimer->Instance->CCR2 = PwmValue;
            LedTimer->Instance->CCR3 = PwmValue;
            LedTimer->Instance->CCR4 = PwmValue;

            if (factor == 0)
                break;

            factor--;

            uint32_t notifiedValue;
            xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, toOsTicks(delayTime));
            if ((notifiedValue & 1U) != 0)
            {
                // restart fading

                restart = true;
                break;
            }
        }
        fadingState = FadingState::Normal;
    }
}