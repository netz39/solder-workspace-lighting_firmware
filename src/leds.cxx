#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"

#include "leds.hpp"

constexpr auto EncoderTimer = &htim1;
constexpr auto LedTimer = &htim2;

extern "C" void fadingTask(void *)
{
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(LedTimer, TIM_CHANNEL_4);

    HAL_TIM_Encoder_Start(EncoderTimer, TIM_CHANNEL_ALL);

    constexpr auto Min = 1;
    constexpr auto Max = 20;
    // constexpr auto CountingStep = 5;

    constexpr auto TimeToFade = 250;
    constexpr auto Diff = Max - Min;
    constexpr auto Delay = TimeToFade / Diff;

    int8_t counter = Max;
    // bool countingUp = true;

    /*
    const auto PwmValue = GammaLUT[counter];
    LedTimer->Instance->CCR1 = PwmValue;
    LedTimer->Instance->CCR2 = PwmValue;
    LedTimer->Instance->CCR3 = PwmValue;
    LedTimer->Instance->CCR4 = PwmValue;
    */

    // vTaskDelay(pdMS_TO_TICKS(1000));
    uint16_t oldEncoderValue = 0;

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        /*
        if (countingUp)
        {
            counter += CountingStep;
            if (counter == Max)
                countingUp = false;
        }
        else
        {
            counter -= CountingStep;
            if (counter == Min)
                countingUp = true;
        }
        */
        /*
        if (counter++ == Max)
        {
            while (true)
            {
                volatile auto EncoderValue = TIM1->CNT;
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        */
        const auto EncoderValue = TIM1->CNT;
        const int8_t Diff = (EncoderValue - oldEncoderValue) / 4;
        if (gcem::abs(Diff) >= 10)
            continue;

        oldEncoderValue = EncoderValue;

        counter += Diff;
        if (counter < Min)
            counter = Min;
        else if (counter > Max)
            counter = Max;

        const auto PwmValue = GammaLUT[counter * 5];
        LedTimer->Instance->CCR1 = PwmValue;
        LedTimer->Instance->CCR2 = PwmValue;
        LedTimer->Instance->CCR3 = PwmValue;
        LedTimer->Instance->CCR4 = PwmValue;
    }
}