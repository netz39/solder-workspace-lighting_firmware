#include "FreeRTOS.h"
#include "queue.h"
#include "spi.h"
#include "task.h"
#include "timers.h"

#include "OledDisplay.hpp"
#include "SSD1306_SPI.hpp"
#include "adc.hpp"
#include "gcem/include/gcem.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"
#include "oled-driver/Display.hpp"
#include "oled-driver/Renderer.hpp"

static SSD1306_SPI ssdi;
static Display display(ssdi);
Renderer renderer(OledWidth, OledPages, display);

extern TaskHandle_t uiHandle;
extern TimerHandle_t ledIdleTimer;

extern "C" void spi1TXCompleteCallback(SPI_HandleTypeDef *);

namespace
{
constexpr auto TaskFrequency = 100.0_Hz;
constexpr auto TimeThresholdToShowCountdown = 9.0_min;

constexpr auto MaximumChars = 22;
char buffer[MaximumChars];

//--------------------------------------------------------------------------------------------------
void notifyRenderTask()
{
    auto higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(uiHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void initDisplay()
{
    HAL_SPI_RegisterCallback(DisplaySpiPeripherie, HAL_SPI_TX_COMPLETE_CB_ID,
                             spi1TXCompleteCallback);

    HAL_GPIO_WritePin(DisplayReset_GPIO_Port, DisplayReset_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(1));
    HAL_GPIO_WritePin(DisplayReset_GPIO_Port, DisplayReset_Pin, GPIO_PIN_SET);
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

} // namespace

//--------------------------------------------------------------------------------------------------
extern "C" void spi1TXCompleteCallback(SPI_HandleTypeDef *)
{
    notifyRenderTask();
}

//--------------------------------------------------------------------------------------------------
void waitForTXComplete()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

//-----------------------------------------------------------------------------
void getTemperaturePrint(const units::si::Temperature &temperature, char *buffer, size_t bufferSize)
{
    if (temperature.getMagnitude() == 0.0f)
        snprintf(buffer, bufferSize, "--°C");
    else
        snprintf(buffer, bufferSize, "%d°C",
                 temperature.getMagnitude<int>(units::si::offset::degC));
}

//--------------------------------------------------------------------------------------------------
void drawDisplay()
{
    renderer.clearAll();

    constexpr auto Space = 47;
    const auto TemperatureTextWidth = renderer.getLineWidth("---°C");
    const auto MinutesTextWidth = renderer.getLineWidth("--min");

    const auto SeperatorColumn1 = (Space + TemperatureTextWidth / 2) / 2;
    const auto SeperatorColumn2 = OledWidth - (TemperatureTextWidth / 2 + Space) / 2;
    const auto SeperatorColumn3 = OledWidth - (MinutesTextWidth + 4);

    if (!xTimerIsTimerActive(ledIdleTimer) || targetLedPercentage == 0)
    {
        xTimerStop(ledIdleTimer, 0);
        snprintf(buffer, MaximumChars, "STANDBY");
    }
    else
    {
        const auto CountdownRemainingTime =
            ticksToTime(xTimerGetExpiryTime(ledIdleTimer) - xTaskGetTickCount());
        const uint8_t RemainingMinutes =
            gcem::ceil(CountdownRemainingTime.getMagnitude(units::si::scale::min));

        if (CountdownRemainingTime <= TimeThresholdToShowCountdown)
        {
            snprintf(buffer, MaximumChars, "%d%% - %dmin", targetLedPercentage, RemainingMinutes);
        }
        else
        {
            snprintf(buffer, MaximumChars, "%dmin", RemainingMinutes);
            renderer.print({OledWidth, 0}, buffer, Renderer::Alignment::Right);

            renderer.drawVerticalLine(SeperatorColumn3, 0, 0);
            renderer.drawHorizontalLine(1, 0, SeperatorColumn3, OledWidth - 1);

            snprintf(buffer, MaximumChars, "%d%%", targetLedPercentage);
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
extern "C" void uiTask(void *)
{
    initDisplay();

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        drawDisplay();
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));
    }
}