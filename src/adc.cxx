#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include "gcem/include/gcem.hpp"
#include "helpers/freertos.hpp"
#include "units/si/frequency.hpp"
#include "units/si/resistance.hpp"
#include "units/si/scalar.hpp"
#include "units/si/temperature.hpp"
#include "units/si/voltage.hpp"

#include <algorithm>
#include <array>

using units::si::Scalar;
using units::si::Temperature;
using units::si::Voltage;

constexpr auto TemperatureChannelCount = 4;
std::array<Temperature, TemperatureChannelCount> ledTemperatures{0.0_degC, 0.0_degC, 0.0_degC,
                                                                 0.0_degC};

extern TaskHandle_t adcHandle;

namespace
{
constexpr auto TotalChannelNumber = TemperatureChannelCount + 1; // vref
constexpr auto VrefChannel = TotalChannelNumber - 1;

auto referenceVoltage = 3.3_V;
constexpr auto AdcTaskFrequency = 100.0_Hz;
constexpr auto AdcResolution = 12;
constexpr auto MaxAdcValue = (1 << AdcResolution) - 1;
constexpr auto AdcPeripherie = &hadc1;

constexpr auto NtcBetaValue = 3380.0f;
constexpr auto NtcNominalTemperature = 25.0_degC;
constexpr auto NtcResistanceAtNominalTemperature = 10_kOhm;
constexpr auto NtcSecondResistor = 10_kOhm;

std::array<uint16_t, TotalChannelNumber> adcResults;

void calculateReferenceVoltage()
{
    const auto VrefIntCalibration = *VREFINT_CAL_ADDR;
    constexpr Voltage CalibrationRefVoltage = 3.0_V;

    referenceVoltage = VrefIntCalibration * CalibrationRefVoltage / adcResults[VrefChannel];
}

template <class T>
constexpr auto physicalQuantityFromAdcResult(const uint16_t adcResult, const T multiplier)
{
    return (adcResult * referenceVoltage * multiplier) / MaxAdcValue;
}

void calibrateAdc()
{
    HAL_ADCEx_Calibration_Start(AdcPeripherie, ADC_SINGLE_ENDED);
}

void startConversion()
{
    HAL_ADC_Start_DMA(AdcPeripherie, reinterpret_cast<uint32_t *>(adcResults.data()),
                      TotalChannelNumber);
}

void waitUntilConversionFinished()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void calculateTemperatures()
{
    for (size_t i = 0; i < TemperatureChannelCount; i++)
    {
        auto DropVoltage = physicalQuantityFromAdcResult(adcResults[i], 1);
        auto NtcResistance =
            (referenceVoltage * NtcSecondResistor - DropVoltage * NtcSecondResistor) / DropVoltage;

        const float LogValue =
            gcem::log((NtcResistance / NtcResistanceAtNominalTemperature).getMagnitude<double>());

        ledTemperatures[i] = units::si::Temperature{
            1 / ((1_ / NtcNominalTemperature).getMagnitude() + (1 / NtcBetaValue) * LogValue)};
    }
}
} // namespace

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *)
{
    auto higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(adcHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);

    // Calling portYIELD_FROM_ISR here is okay even if it is not the last statement of the ISR.
    // Note that this does not necessarily hold true for other architectures.
}

extern "C" void adcTask(void *)
{
    calibrateAdc();
    auto lastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        startConversion();
        waitUntilConversionFinished();

        calculateReferenceVoltage();
        calculateTemperatures();

        vTaskDelayUntil(&lastWakeTime, toOsTicks(AdcTaskFrequency));
    }
}