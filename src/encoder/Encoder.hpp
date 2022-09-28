#pragma once

#include "tim.h"

#include "core/SafeAssert.h"
#include "led_control/LedFading.hpp"
#include "wrappers/Task.hpp"

#include "helpers/freertos.hpp"

class Encoder : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    Encoder(TIM_HandleTypeDef *encoderTimer, bool &isOverTemperature, LedFading &ledFading)
        : TaskWithMemberFunctionBase("encoderTask", 512, osPriorityNormal2), //
          encoderTimer(encoderTimer),                                        //
          isOverTemperature(isOverTemperature),                              //
          ledFading(ledFading)
    {
        SafeAssert(encoderTimer != nullptr);
    }

    static constexpr auto TaskFrequency = 50.0_Hz;

protected:
    [[noreturn]] void taskMain() override;

private:
    TIM_HandleTypeDef *encoderTimer = nullptr;
    bool &isOverTemperature;
    LedFading &ledFading;
};