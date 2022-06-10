#include "FreeRTOS.h"
#include "main.h"
#include "task.h"
#include "timers.h"

#include "Button.hpp"
#include "Gpio.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"

extern TimerHandle_t ledIdleTimer;
extern FadingState fadingState;
extern TaskHandle_t fadingHandle;
extern void resetLedIdleTimeout();
extern FadingState fadingState;

using units::si::Time;
using util::Button;
using util::Gpio;

namespace
{
constexpr auto ButtonSamplingInterval = 15.0_ms;
constexpr auto LongPressTime = 500.0_ms;

void encoderButtonCallback(Button::Action action)
{
    if (action == Button::Action::ShortPress)
    {
        resetLedIdleTimeout();
        targetLedPercentage = DefaultPercentage;
        fadingState = FadingState::Normal;
        xTaskNotify(fadingHandle, 1U, eSetBits);
    }

    else if (action == Button::Action::LongPress)
    {
        xTimerStop(ledIdleTimer, 0);
        targetLedPercentage = MinPercentage;
        fadingState = FadingState::Normal;
        xTaskNotify(fadingHandle, 1U, eSetBits);
    }
}

Button encoderButton({EncoderButton_GPIO_Port, EncoderButton_Pin}, encoderButtonCallback,
                     LongPressTime);

} // namespace

extern "C" void buttonUpdateTask(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        encoderButton.update(ButtonSamplingInterval);

        vTaskDelayUntil(&lastWakeTime, toOsTicks(ButtonSamplingInterval));
    }
}
