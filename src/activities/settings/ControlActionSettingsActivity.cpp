#include "ControlActionSettingsActivity.h"

#include <HalTiltSensor.h>

#include "ControlActionSettings.h"
#include "activities/RenderLock.h"
#include "components/UITheme.h"

void ControlActionSettingsActivity::onEnter() {
  Activity::onEnter();

  selectedIndex = 0;
  visibleItemCount = halTiltSensor.isAvailable() ? MenuItem::ITEM_COUNT : MenuItem::LEFT_TILT;
  requestUpdate();
}

void ControlActionSettingsActivity::onExit() { Activity::onExit(); }

void ControlActionSettingsActivity::loop() {}

void ControlActionSettingsActivity::render(RenderLock&&) {
  auto metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight},
                 tr(STR_CUSTOMIZE_CONTROL_ACTIONS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;

  // Draw button hints
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_TOGGLE), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}

void ControlActionSettingsActivity::handleSelection() {
  switch (selectedIndex) {
    case BACK_BUTTON:
      SETTINGS.statusBarChapterPageCount = (SETTINGS.statusBarChapterPageCount + 1) % 2;
      break;
    case CONFIRM_BUTTON:
      SETTINGS.statusBarBookProgressPercentage = (SETTINGS.statusBarBookProgressPercentage + 1) % 2;
      break;
    case LEFT_BUTTON:
      SETTINGS.statusBarProgressBar = (SETTINGS.statusBarProgressBar + 1) % PROGRESS_BAR_ITEMS;
      break;
    case RIGHT_BUTTON:
      SETTINGS.statusBarProgressBarThickness =
          (SETTINGS.statusBarProgressBarThickness + 1) % PROGRESS_BAR_THICKNESS_ITEMS;
      break;
    case UP_BUTTON:
      SETTINGS.statusBarTitle = (SETTINGS.statusBarTitle + 1) % TITLE_ITEMS;
      break;
    case DOWN_BUTTON:
      SETTINGS.statusBarBattery = (SETTINGS.statusBarBattery + 1) % 2;
      break;
    case POWER_BUTTON:
      SETTINGS.xtcStatusBarMode = (SETTINGS.xtcStatusBarMode + 1) % XTC_STATUS_BAR_ITEMS;
      break;
    case LEFT_TILT:
      SETTINGS.statusBarClock = (SETTINGS.statusBarClock + 1) % 2;
      break;
    case RIGHT_TILT:
      SETTINGS.clockFormat = (SETTINGS.clockFormat + 1) % CLOCK_FORMAT_ITEMS;
      break;
    default:
      return;
  }

  CONTROLACTIONSETTINGS.saveToFile();
}