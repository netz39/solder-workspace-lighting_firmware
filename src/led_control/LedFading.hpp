#pragma once

#include "tim.h"

#include "core/SafeAssert.h"
#include "helpers/freertos.hpp"
#include "timers.h"
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

    LedFading(TIM_HandleTypeDef *ledTimer, void (*timeoutCallback)(TimerHandle_t xTimer))
        : TaskWithMemberFunctionBase("ledFadingTask", 512, osPriorityNormal4), //
          ledTimer(ledTimer),                                                  //
          timeoutCallback(timeoutCallback)
    {
        SafeAssert(ledTimer != nullptr);

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
    [[noreturn]] void taskMain() override;

private:
    TIM_HandleTypeDef *ledTimer = nullptr;
    TimerHandle_t ledIdleTimer = nullptr;

    FadingState fadingState = FadingState::Normal;

    uint8_t currentLedPercentage = MinPercentage;
    uint8_t targetLedPercentage = DefaultPercentage;

public:
    void (*timeoutCallback)(TimerHandle_t);
};