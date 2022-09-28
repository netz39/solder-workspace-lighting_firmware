#include "FreeRTOS.h"
#include "task.h"

#include "OledDisplay.hpp"
#include "gcem/include/gcem.hpp"
#include "led_control/LedFading.hpp"

//--------------------------------------------------------------------------------------------------
void OledDisplay::notifyRenderTaskFromISR()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyGiveFromISR(&higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void OledDisplay::initDisplay()
{
    displayReset.write(false);
    vTaskDelay(pdMS_TO_TICKS(1));
    displayReset.write(true);
    vTaskDelay(pdMS_TO_TICKS(1));

    display.setDisplayState(Display::DisplayState::Off);
    display.setMultiplexRatio(OledPages * 8 - 1);
    display.setComPinConfig(false, false);
    display.setContrastControl(0xFF);
    display.setDisplayClockDivide(0, 0x8);
    display.setSegmentRemap(false);
    display.setComOutputMode(Display::ComMode::Normal);
    display.setPrechargingPeriod(1, 0xF);
    display.setChargePump(true);

    display.setMemoryAddressingMode(Display::AddressingMode::Horizontal);
    display.setPageAddress(0, OledPages - 1);   // pages from 0 to 3
    display.setColumnAddress(0, OledWidth - 1); // columns from 0 to 127

    display.setDisplayState(Display::DisplayState::On);
}

//--------------------------------------------------------------------------------------------------
void OledDisplay::waitForTXComplete()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

//-----------------------------------------------------------------------------
void OledDisplay::getTemperaturePrint(const units::si::Temperature &temperature, char *buffer,
                                      size_t bufferSize)
{
    if (temperature.getMagnitude() == 0.0f)
        snprintf(buffer, bufferSize, "--°C");
    else
        snprintf(buffer, bufferSize, "%d°C",
                 temperature.getMagnitude<int>(units::si::offset::degC));
}

//--------------------------------------------------------------------------------------------------
void OledDisplay::drawDisplay()
{
    renderer.clearAll();

    constexpr auto Space = 47;
    const auto TemperatureTextWidth = renderer.getLineWidth("---°C");
    const auto MinutesTextWidth = renderer.getLineWidth("--min");

    const auto SeperatorColumn1 = (Space + TemperatureTextWidth / 2) / 2;
    const auto SeperatorColumn2 = OledWidth - (TemperatureTextWidth / 2 + Space) / 2;
    const auto SeperatorColumn3 = OledWidth - (MinutesTextWidth + 4);

    if (!ledFading.isIdleTimerActive() || ledFading.getTargetPercentage() == 0)
    {
        ledFading.stopIdleTimer(0);
        snprintf(buffer, MaximumChars, "STANDBY");
    }
    else
    {
        const auto CountdownRemainingTime =
            ticksToTime(ledFading.getIdleExpiryTime() - xTaskGetTickCount());
        const uint8_t RemainingMinutes =
            gcem::ceil(CountdownRemainingTime.getMagnitude(units::si::scale::min));

        if (CountdownRemainingTime <= TimeThresholdToShowCountdown)
        {
            snprintf(buffer, MaximumChars, "%d%% - %dmin", ledFading.getTargetPercentage(),
                     RemainingMinutes);
        }
        else
        {
            snprintf(buffer, MaximumChars, "%dmin", RemainingMinutes);
            renderer.print({OledWidth, 0}, buffer, Renderer::Alignment::Right);

            renderer.drawVerticalLine(SeperatorColumn3, 0, 0);
            renderer.drawHorizontalLine(1, 0, SeperatorColumn3, OledWidth - 1);

            snprintf(buffer, MaximumChars, "%d%%", ledFading.getTargetPercentage());
        }
    }
    renderer.print({OledWidth / 2, 0}, buffer, Renderer::Alignment::Center, 2);

    constexpr auto Row = 4;
    renderer.drawHorizontalLine(2, Row);
    renderer.drawVerticalLine(SeperatorColumn1, 2, 3, Row);
    renderer.drawVerticalLine(OledWidth / 2, 2, 3, Row);
    renderer.drawVerticalLine(SeperatorColumn2, 2, 3, Row);

    getTemperaturePrint(ledTemperatures[0], buffer, MaximumChars);
    renderer.print({0, 3}, buffer, Renderer::Alignment::Left);

    getTemperaturePrint(ledTemperatures[1], buffer, MaximumChars);
    renderer.print({Space, 3}, buffer, Renderer::Alignment::Center);

    getTemperaturePrint(ledTemperatures[2], buffer, MaximumChars);
    renderer.print({OledWidth - Space, 3}, buffer, Renderer::Alignment::Center);

    getTemperaturePrint(ledTemperatures[3], buffer, MaximumChars);
    renderer.print({OledWidth, 3}, buffer, Renderer::Alignment::Right);

    renderer.render();
}

//--------------------------------------------------------------------------------------------------
void OledDisplay::taskMain()
{
    initDisplay();

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        drawDisplay();
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));
    }
}