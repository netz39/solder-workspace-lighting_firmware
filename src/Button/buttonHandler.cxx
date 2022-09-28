#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "buttonHandler.hpp"
#include "leds.hpp"

void ButtonHandler::encoderButtonCallback(util::Button::Action action)
{
    if (action == util::Button::Action::ShortPress)
    {
        ledFading.resetLedIdleTimeout();
        ledFading.setTargetPercentage(LedFading::DefaultPercentage);
        ledFading.setFadingState(LedFading::FadingState::Normal);
        ledFading.notify(1U, util::wrappers::NotifyAction::SetBits);
    }

    else if (action == util::Button::Action::LongPress)
    {
        ledFading.stopIdleTimer(0);
        ledFading.setTargetPercentage(LedFading::MinPercentage);
        ledFading.setFadingState(LedFading::FadingState::Normal);
        ledFading.notify(1U, util::wrappers::NotifyAction::SetBits);
    }
}

[[noreturn]] void ButtonHandler::taskMain()
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        encoderButton.update(ButtonSamplingInterval);

        vTaskDelayUntil(&lastWakeTime, toOsTicks(ButtonSamplingInterval));
    }
}
