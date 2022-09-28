#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include "adc.hpp"
#include "gcem/include/gcem.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"

#include <algorithm>

using units::si::Scalar;
using units::si::Temperature;
using units::si::Voltage;
using util::wrappers::NotifyAction;

void AnalogToDigital::taskMain()
{
    calibrateAdc();
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        startConversion();
        waitUntilConversionFinished();

        calculateReferenceVoltage();
        calculateTemperatures();

        checkOverTemperature();

        vTaskDelayUntil(&lastWakeTime, toOsTicks(AdcTaskFrequency));
    }
}

void AnalogToDigital::calculateReferenceVoltage()
{
    const auto VrefIntCalibration = *VREFINT_CAL_ADDR;
    constexpr Voltage CalibrationRefVoltage = 3.0_V;

    referenceVoltage = VrefIntCalibration * CalibrationRefVoltage / adcResults[VrefChannel];
}

void AnalogToDigital::calibrateAdc()
{
    HAL_ADCEx_Calibration_Start(peripherie, ADC_SINGLE_ENDED);
}

void AnalogToDigital::startConversion()
{
    HAL_ADC_Start_DMA(peripherie, reinterpret_cast<uint32_t *>(adcResults.data()),
                      TotalChannelNumber);
}

void AnalogToDigital::waitUntilConversionFinished()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void AnalogToDigital::conversionCompleteCallback()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void AnalogToDigital::calculateTemperatures()
{
    for (size_t i = 0; i < TemperatureChannelCount; i++)
    {
        if (adcResults[i] == 0)
        {
            ledTemperatures[i] = units::si::Temperature(0);
            continue;
        }

        const auto DropVoltage = physicalQuantityFromAdcResult(adcResults[i], 1);
        const auto NtcResistance =
            (referenceVoltage * NtcSecondResistor - DropVoltage * NtcSecondResistor) / DropVoltage;

        const float LogValue =
            gcem::log((NtcResistance / NtcResistanceAtNominalTemperature).getMagnitude<double>());

        const auto NewTemperatureValue = units::si::Temperature{
            1 / ((1_ / NtcNominalTemperature).getMagnitude() + (1 / NtcBetaValue) * LogValue)};

        updateFastLowpass(ledTemperatures[i], NewTemperatureValue, 16);
    }
}

void AnalogToDigital::checkOverTemperature()
{
    Temperature maximumTemperature =
        *std::max_element(ledTemperatures.begin(), ledTemperatures.end());

    if (!isOverTemperature)
    {
        if (maximumTemperature >= MaximumTemperatureUpperThreshold)
        {
            isOverTemperature = true;
            if (ledFading.getTargetPercentage() > LedFading::DefaultPercentage)
            {
                ledFading.setTargetPercentage(LedFading::DefaultPercentage);
                ledFading.setFadingState(LedFading::FadingState::Normal);
                ledFading.notify(1U, util::wrappers::NotifyAction::SetBits);
            }
        }
    }
    else if (maximumTemperature <= MaximumTemperatureLowerThreshold)
    {
        isOverTemperature = false;
    }
}