#include "ControlActionSettings.h"

#include <CrossPointSettings.h>
#include <GfxRenderer.h>

#include "MappedInputManager.h"

void ControlActionSettings::getAllowSleepAt(unsigned long sleepAt) { allowSleepAt = sleepAt; };

void ControlActionSettings::getRenderer(GfxRenderer& grenderer) { renderer = grenderer; }

void ControlActionSettings::takeScreenshotControlAction(GfxRenderer& renderer) {
  RenderLock lock;
  ScreenshotUtil::takeScreenshot(renderer);
  return;
}

void ControlActionSettings::sleepControlAction() {
  if (millis() >= allowSleepAt && gpio.isPressed(HalGPIO::BTN_POWER) &&
      gpio.getPowerButtonHeldTime() > SETTINGS.getPowerButtonDuration()) {
    // If the screenshot combination is potentially being pressed, don't sleep
    if (gpio.isPressed(HalGPIO::BTN_DOWN)) {
      return;
    }
    enterDeepSleep();
    // This should never be hit as `enterDeepSleep` calls esp_deep_sleep_start
    return;
  }
}

void ControlActionSettings::forceRefreshControlAction() {
  if (SETTINGS.shortPwrBtn == CrossPointSettings::SHORT_PWRBTN::FORCE_REFRESH &&
      mappedInputManager.wasReleased(MappedInputManager::Button::Power)) {
    LOG_DBG("MAIN", "Manual screen refresh triggered");
    RenderLock lock;
    renderer.displayBuffer(HalDisplay::HALF_REFRESH);
  }
}