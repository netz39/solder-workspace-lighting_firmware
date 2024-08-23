#pragma once

#include "tim.h"

#include "core/SafeAssert.h"
#include "helpers/freertos.hpp"
#include "timers.h"
#include "util/PwmOutput.hpp"
#include "util/led/GammaCorrection.hpp"
#include "wrappers/Task.hpp"

class LedFading : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto NumberOfLeds = 4;
    static constexpr auto MinPercentage = 0;
    static constexpr auto MaxPercentage = 100;
    static constexpr auto DefaultPercentage = 80;

    static constexpr auto TaskFrequency = 100.0_Hz;

    static constexpr auto TimeToFade = 100.0_ms;
    static constexpr auto TimeToFadeOff = 5.0_s;
    static constexpr auto LedIdleTimout = 45.0_min;

    enum class FadingState
    {
        Normal,
        Standby
    };

    LedFading(TIM_HandleTypeDef *ledTimerPeripherie, void (*timeoutCallback)(TimerHandle_t xTimer))
        : TaskWithMemberFunctionBase("ledFadingTask", 512, osPriorityNormal4), //
          ledTimerPeripherie(ledTimerPeripherie),                              //
          timeoutCallback(timeoutCallback)
    {
        SafeAssert(ledTimerPeripherie != nullptr);

        ledIdleTimer = xTimerCreate("ledIdleTimeout", toOsTicks(LedIdleTimout), pdFALSE, nullptr,
                                    timeoutCallback);

        SafeAssert(ledIdleTimer != nullptr);
    }

    void setTargetPercentage(uint8_t percentage)
    {
        targetLedPercentage = percentage;
    }

    [[nodiscard]] uint8_t getTargetPercentage() const
    {
        return targetLedPercentage;
    }

    void setFadingState(FadingState newState)
    {
        fadingState = newState;
    }

    [[nodiscard]] bool isIdleTimerActive()
    {
        return xTimerIsTimerActive(ledIdleTimer);
    }

    [[nodiscard]] TickType_t getIdleExpiryTime()
    {
        return xTimerGetExpiryTime(ledIdleTimer);
    }

    void stopIdleTimer(const TickType_t xTicksToWait)
    {
        xTimerStop(ledIdleTimer, xTicksToWait);
    }

    void resetLedIdleTimeout();
    void onLedIdleTimeout();

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    TIM_HandleTypeDef *ledTimerPeripherie = nullptr;
    TimerHandle_t ledIdleTimer = nullptr;

    FadingState fadingState = FadingState::Normal;

    uint8_t currentLedPercentage = MinPercentage;
    uint8_t targetLedPercentage = DefaultPercentage;

    static constexpr auto PwmResolution = 11;
    static constexpr util::led::pwm::GammaCorrection<PwmResolution> GammaLut{};

    std::array<util::PwmOutput<PwmResolution>, NumberOfLeds> ledSpotArray{
        util::PwmOutput<PwmResolution>{ledTimerPeripherie, TIM_CHANNEL_1},
        util::PwmOutput<PwmResolution>{ledTimerPeripherie, TIM_CHANNEL_2},
        util::PwmOutput<PwmResolution>{ledTimerPeripherie, TIM_CHANNEL_3},
        util::PwmOutput<PwmResolution>{ledTimerPeripherie, TIM_CHANNEL_4}};

    // map percentage (0-100) to pwm value (0-2047)
    uint16_t mapPercentageToPwmValue(uint8_t percentage);

    // map pwm value (0-2047) to percentage (0-100)
    uint8_t mapPwmValueToPercentage(uint16_t pwmValue);

public:
    void (*timeoutCallback)(TimerHandle_t);
};