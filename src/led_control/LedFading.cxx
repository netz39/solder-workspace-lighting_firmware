#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "timers.h"

#include "helpers/freertos.hpp"
#include "led_control/LedFading.hpp"
#include "units/si/frequency.hpp"

#include <climits>

void LedFading::taskMain(void *)
{
    resetLedIdleTimeout();
    bool restart = true;

    while (true)
    {
        if (!restart)
            notifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;

        int16_t pwmValueDifference = mapPercentageToPwmValue(currentLedPercentage) -
                                     mapPercentageToPwmValue(targetLedPercentage);
        uint16_t numberOfSteps = gcem::abs(pwmValueDifference);

        units::si::Time fadeTime = 0.0_s;
        switch (fadingState)
        {
        case FadingState::Normal:
            fadeTime = TimeToFade;
            break;

        case FadingState::Standby:
            fadeTime = TimeToFadeOff;
            break;
        }

        size_t stepSize = 1;
        auto delayPerStep = fadeTime / numberOfSteps;
        const size_t PossibleSteps = toOsTicks(fadeTime);

        if (numberOfSteps > PossibleSteps)
        {
            // increase step size to fit into task minimum delay of 1ms
            const float StepSizeFactor = static_cast<float>(numberOfSteps) / PossibleSteps;
            stepSize = ceil(StepSizeFactor); // round up to next integer

            // adjust delay per step to frame given fadeTime
            delayPerStep = 1.0_ms * (stepSize / StepSizeFactor);

            // recalculate number of steps to align it with step size
            numberOfSteps -= numberOfSteps % stepSize;
        }

        const bool IsDecreasingPwm = pwmValueDifference > 0;

        // set start point aligned to step size
        uint16_t currentPwmValue = mapPercentageToPwmValue(targetLedPercentage) -
                                   (IsDecreasingPwm ? -numberOfSteps : numberOfSteps);

        while (true)
        {
            currentPwmValue += IsDecreasingPwm ? -stepSize : stepSize;
            currentLedPercentage = mapPwmValueToPercentage(currentPwmValue);

            for (auto &ledSpot : ledSpotArray)
                ledSpot.setPwmValue(GammaCorrection.LookUpTable[currentPwmValue]);

            if (currentPwmValue == mapPercentageToPwmValue(targetLedPercentage))
                break;

            uint32_t notifiedValue;
            notifyWait(0, ULONG_MAX, &notifiedValue, toOsTicks(delayPerStep));
            if ((notifiedValue & 1U) != 0)
            {
                // restart fading
                restart = true;
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
uint16_t LedFading::mapPercentageToPwmValue(uint8_t percentage)
{
    return (std::min(percentage, (uint8_t)100) * GammaCorrection.MaxResolutionValue) / 100;
}

// ----------------------------------------------------------------------------
uint8_t LedFading::mapPwmValueToPercentage(uint16_t pwmValue)
{
    return (pwmValue * 100) / GammaCorrection.MaxResolutionValue;
}

// ----------------------------------------------------------------------------
void LedFading::onLedIdleTimeout()
{
    fadingState = FadingState::Standby;
    targetLedPercentage = MinPercentage;
    notify(1U, util::wrappers::NotifyAction::SetBits);
}

// ----------------------------------------------------------------------------
void LedFading::resetLedIdleTimeout()
{
    xTimerReset(ledIdleTimer, 0);
}