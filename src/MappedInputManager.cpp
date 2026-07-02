#include "MappedInputManager.h"

#include <GfxRenderer.h>

#include "CrossPointSettings.h"

bool MappedInputManager::isNavDirectionSwapped() const {
  // Key the swap on the orientation the screen is *actually* rendered at, not the persisted reader
  // setting. The reader (and its modal menus) render rotated, so navigation/labels flip there; the
  // home and settings UI render in portrait, so they never flip even when a rotated reader is configured.
  const auto orientation = renderer.getOrientation();
  return SETTINGS.frontButtonFollowOrientation &&
         (orientation == GfxRenderer::PortraitInverted || orientation == GfxRenderer::LandscapeCounterClockwise);
}

bool MappedInputManager::mapButton(const Button button, bool (HalGPIO::*fn)(uint8_t) const) const {
  const auto sideLayout = SETTINGS.sideButtonLayout;

  switch (button) {
    case Button::Back:
      // Logical Back maps to user-configured front button.
      return (gpio.*fn)(SETTINGS.frontButtonBack);
    case Button::Confirm:
      // Logical Confirm maps to user-configured front button.
      return (gpio.*fn)(SETTINGS.frontButtonConfirm);
    case Button::Left:
      // Logical Left maps to user-configured front button.
      return (gpio.*fn)(SETTINGS.frontButtonLeft);
    case Button::Right:
      // Logical Right maps to user-configured front button.
      return (gpio.*fn)(SETTINGS.frontButtonRight);
    case Button::Up:
      // Side buttons remain fixed for Up/Down.
      return (gpio.*fn)(HalGPIO::BTN_UP);
    case Button::Down:
      // Side buttons remain fixed for Up/Down.
      return (gpio.*fn)(HalGPIO::BTN_DOWN);
    case Button::Power:
      // Power button bypasses remapping.
      return (gpio.*fn)(HalGPIO::BTN_POWER);
    case Button::PageBack:
      // Reader page navigation uses side buttons and can be swapped via settings.
      switch (sideLayout) {
        case CrossPointSettings::PREV_NEXT:
          return (gpio.*fn)(HalGPIO::BTN_UP);
        case CrossPointSettings::NEXT_PREV:
          return (gpio.*fn)(HalGPIO::BTN_DOWN);
        case CrossPointSettings::SIDE_BUTTONS_DISABLED:
        default:
          return false;
      }
    case Button::PageForward:
      // Reader page navigation uses side buttons and can be swapped via settings.
      switch (sideLayout) {
        case CrossPointSettings::PREV_NEXT:
          return (gpio.*fn)(HalGPIO::BTN_DOWN);
        case CrossPointSettings::NEXT_PREV:
          return (gpio.*fn)(HalGPIO::BTN_UP);
        case CrossPointSettings::SIDE_BUTTONS_DISABLED:
        default:
          return false;
      }
    case Button::NavNext:
      // Logical "next item" navigation: side Down + front Right, with the control axis flipped in
      // INVERTED / LANDSCAPE_CCW (frontButtonFollowOrientation) so it matches the rotated hint labels.
      return isNavDirectionSwapped() ? (mapButton(Button::Up, fn) || mapButton(Button::Left, fn))
                                     : (mapButton(Button::Down, fn) || mapButton(Button::Right, fn));
    case Button::NavPrevious:
      // Logical "previous item" navigation: side Up + front Left, axis-flipped in the same orientations.
      return isNavDirectionSwapped() ? (mapButton(Button::Down, fn) || mapButton(Button::Right, fn))
                                     : (mapButton(Button::Up, fn) || mapButton(Button::Left, fn));
  }

  return false;
}

bool MappedInputManager::wasPressed(const Button button) const { return mapButton(button, &HalGPIO::wasPressed); }

bool MappedInputManager::wasReleased(const Button button) const { return mapButton(button, &HalGPIO::wasReleased); }

bool MappedInputManager::isPressed(const Button button) const { return mapButton(button, &HalGPIO::isPressed); }

bool MappedInputManager::wasAnyPressed() const { return gpio.wasAnyPressed(); }

bool MappedInputManager::wasAnyReleased() const { return gpio.wasAnyReleased(); }

unsigned long MappedInputManager::getHeldTime() const { return gpio.getHeldTime(); }

MappedInputManager::Labels MappedInputManager::mapLabels(const char* back, const char* confirm, const char* previous,
                                                         const char* next) const {
  // Swap previous/next labels to match the page turn direction swap in INVERTED and LANDSCAPE_CCW.
  const bool swapLabels = isNavDirectionSwapped();
  const char* leftLabel = swapLabels ? next : previous;
  const char* rightLabel = swapLabels ? previous : next;

  // Build the label order based on the configured hardware mapping.
  auto labelForHardware = [&](uint8_t hw) -> const char* {
    // Compare against configured logical roles and return the matching label.
    if (hw == SETTINGS.frontButtonBack) {
      return back;
    }
    if (hw == SETTINGS.frontButtonConfirm) {
      return confirm;
    }
    if (hw == SETTINGS.frontButtonLeft) {
      return leftLabel;
    }
    if (hw == SETTINGS.frontButtonRight) {
      return rightLabel;
    }
    return "";
  };

  return {labelForHardware(HalGPIO::BTN_BACK), labelForHardware(HalGPIO::BTN_CONFIRM),
          labelForHardware(HalGPIO::BTN_LEFT), labelForHardware(HalGPIO::BTN_RIGHT)};
}

int MappedInputManager::getPressedRawButton() const {
  // Scan the raw buttons in hardware order.
  // This bypasses remapping so the remap activity can capture physical presses.
  if (gpio.wasPressed(HalGPIO::BTN_BACK)) {
    return HalGPIO::BTN_BACK;
  }
  if (gpio.wasPressed(HalGPIO::BTN_CONFIRM)) {
    return HalGPIO::BTN_CONFIRM;
  }
  if (gpio.wasPressed(HalGPIO::BTN_LEFT)) {
    return HalGPIO::BTN_LEFT;
  }
  if (gpio.wasPressed(HalGPIO::BTN_RIGHT)) {
    return HalGPIO::BTN_RIGHT;
  }
  if (gpio.wasPressed(HalGPIO::BTN_UP)) {
    return HalGPIO::BTN_UP;
  }
  if (gpio.wasPressed(HalGPIO::BTN_DOWN)) {
    return HalGPIO::BTN_DOWN;
  }
  if (gpio.wasPressed(HalGPIO::BTN_POWER)) {
    return HalGPIO::BTN_POWER;
  }
  return -1;
}

StrId MappedInputManager::processInputButton(std::map<std::set<Button>, std::vector<ControlAction>> buttonMap) const {
  static bool buttonPressedActivityEnter = false;
  static bool isActionExecuting = false;

  static bool isShortPressQueued = false;
  static bool isLongPressHeld = false;
  static bool isRepeatWhenHeld = false;
  std::set<Button> isPressedButtons = {};
  std::set<Button> wasPressedButtons = {};
  std::set<Button> wasReleasedButtons = {};
  std::map<PRESS_TYPE, ControlAction> isPressedActionMap = {};
  std::map<PRESS_TYPE, ControlAction> wasPressedTypeMap = {};
  std::map<PRESS_TYPE, ControlAction> wasReleasedActionMap = {};
  bool hasShortPress = false;
  bool hasLongPress = false;
  bool hasDoublePress = false;
  static ControlAction currentlyExecutingAction;
  static uint16_t shortPressQueuedStartTime;
  static uint16_t actionExecuteTime;
  static std::set<Button> lastButtonsShortPressed;
  static std::set<Button> lastButtonsIsPressed;

  // When entering an activity, a button must have been pressed at least once before continuing.
  // This is so continuing to hold a button when exiting from a previous activity won't accidentally
  // cause a control action to be executed when entering another activity.
  if (MappedInputManager::wasAnyPressed() && !buttonPressedActivityEnter) {
    buttonPressedActivityEnter = true;
  } else if (!buttonPressedActivityEnter) {
    return;
  }

  // When no action is being executed, based on which buttons have been pressed/released check that activation
  // condition for actions have been fulfilled. If fulfilled, set the action as the currently executing action.
  // Figure out which action to execute by determining which buttons are being interacted with and then checking
  // if press type condition has been fulfilled.
  if (!isActionExecuting) {
    if (!MappedInputManager::isAnyPressed() && !MappedInputManager::wasAnyReleased() && !isShortPressQueued) {
      return;
    }

    if (MappedInputManager::isAnyPressed()) {
      isPressedButtons = getButtonsIsPressed();
      for (ControlAction action : buttonMap[isPressedButtons]) {
        for (ButtonAndPressTypeSettings setting : action.buttonPressTypeSettings) {
          if (setting.buttonAndPressType.buttons == isPressedButtons) {
            isPressedActionMap.emplace(setting.buttonAndPressType.pressType, action);
            if (setting.buttonAndPressType.pressType == PRESS_TYPE::DOUBLE) {
              hasDoublePress = true;
            } else if (setting.buttonAndPressType.pressType == PRESS_TYPE::LONG) {
              hasLongPress = true;
            }
          }
        }
      }
      // If button(s) has been pressed in previous and current update, that means button(s) is being held.
      if (lastButtonsIsPressed == isPressedButtons) {
        isLongPressHeld = true;
      } else {
        isLongPressHeld = false;
      }
      lastButtonsIsPressed = isPressedButtons;
    }

    if (MappedInputManager::wasAnyPressed()) {
      wasPressedButtons = getButtonsWasPressed();
      for (ControlAction action : buttonMap[wasPressedButtons]) {
        for (ButtonAndPressTypeSettings setting : action.buttonPressTypeSettings) {
          if (setting.buttonAndPressType.buttons == wasPressedButtons) {
            wasPressedTypeMap.emplace(setting.buttonAndPressType.pressType, action);
            if (setting.buttonAndPressType.pressType == PRESS_TYPE::DOUBLE) {
              hasDoublePress = true;
            } else if (setting.buttonAndPressType.pressType == PRESS_TYPE::LONG) {
              hasLongPress = true;
            }
          }
        }
      }
    }

    if (MappedInputManager::wasAnyReleased()) {
      wasReleasedButtons = getButtonsWasReleased();
      for (ControlAction action : buttonMap[wasReleasedButtons]) {
        for (ButtonAndPressTypeSettings setting : action.buttonPressTypeSettings) {
          if (setting.buttonAndPressType.buttons == wasReleasedButtons) {
            wasReleasedActionMap.emplace(setting.buttonAndPressType.pressType, action);
            if (setting.buttonAndPressType.pressType == PRESS_TYPE::DOUBLE) {
              hasDoublePress = true;
            } else if (setting.buttonAndPressType.pressType == PRESS_TYPE::LONG) {
              hasLongPress = true;
            }
          }
        }
      }
      isLongPressHeld = false;
    }

    // Activation conditions are fulfilled differently depending on the press types that have been associated with a set
    // of button(s). For example: If there's only a Short Press, then pressing the button will immediately trigger the
    // action to turn the page. If there's both a Short Press and Double Press and button has not been pressed within
    // doublePressDurationMSec of the first press, then Short Press to trigger the action to turn the page will
    // activate.
    if (hasDoublePress) {
      if (hasLongPress) {
        if (!isShortPressQueued) {
          if (isLongPressHeld && getHeldTime() > isPressedActionMap[PRESS_TYPE::LONG].longPressDurationMSec) {
            currentlyExecutingAction = isPressedActionMap[PRESS_TYPE::LONG];
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::LONG) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isLongPressHeld = false;
          } else if (MappedInputManager::wasAnyReleased()) {
            isShortPressQueued = true;
            shortPressQueuedStartTime = millis();
            lastButtonsShortPressed = wasReleasedButtons;
          }
        } else {
          if (wasPressedButtons == lastButtonsShortPressed) {
            currentlyExecutingAction = wasPressedTypeMap[PRESS_TYPE::DOUBLE];
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::DOUBLE) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isShortPressQueued = false;
            lastButtonsShortPressed.clear();
          } else if ((millis() - shortPressQueuedStartTime) >
                     isPressedActionMap[PRESS_TYPE::DOUBLE].doublePressDurationMSec) {
            currentlyExecutingAction = wasReleasedActionMap[PRESS_TYPE::SHORT];
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::SHORT) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isShortPressQueued = false;
            lastButtonsShortPressed.clear();
          }
        }
      } else {
        if (!isShortPressQueued) {
          if (MappedInputManager::wasAnyPressed()) {
            isShortPressQueued = true;
            shortPressQueuedStartTime = millis();
            lastButtonsShortPressed = wasPressedButtons;
          }
        } else {
          if (wasPressedButtons == lastButtonsShortPressed) {
            currentlyExecutingAction = isPressedActionMap[PRESS_TYPE::DOUBLE];
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::DOUBLE) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isShortPressQueued = false;
            lastButtonsShortPressed.clear();
          } else if ((millis() - shortPressQueuedStartTime) >
                     isPressedActionMap[PRESS_TYPE::DOUBLE].doublePressDurationMSec) {
            currentlyExecutingAction = wasPressedTypeMap[PRESS_TYPE::SHORT];
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::SHORT) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isShortPressQueued = false;
            lastButtonsShortPressed.clear();
          }
        }
      }
    } else {
      if (hasLongPress) {
        if (!isShortPressQueued) {
          if (isLongPressHeld && getHeldTime() > isPressedActionMap[PRESS_TYPE::LONG].longPressDurationMSec) {
            currentlyExecutingAction = isPressedActionMap[PRESS_TYPE::LONG];
            LOG_DBG("INPUT", "LONG %i", getHeldTime());
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::LONG) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
            isLongPressHeld = false;
          } else if (MappedInputManager::wasAnyReleased()) {
            currentlyExecutingAction = wasReleasedActionMap[PRESS_TYPE::SHORT];
            LOG_DBG("INPUT", "SHORT");
            for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
              if (setting.buttonAndPressType.pressType == PRESS_TYPE::SHORT) {
                isRepeatWhenHeld = setting.isRepeatWhenHeld;
                break;
              }
            }
          }
        }
      } else {
        if (MappedInputManager::wasAnyPressed()) {
          currentlyExecutingAction = wasPressedTypeMap[PRESS_TYPE::SHORT];
          LOG_DBG("INPUT", "SHORT");
          for (ButtonAndPressTypeSettings setting : currentlyExecutingAction.buttonPressTypeSettings) {
            if (setting.buttonAndPressType.pressType == PRESS_TYPE::SHORT) {
              isRepeatWhenHeld = setting.isRepeatWhenHeld;
              break;
            }
          }
        }
      }
    }
  }

  // When the currently executing action is set, execute immediately for the first time, and it may be repeatedly
  // executed. Exit the executing action when button(s) is no longer being pressed.
  if (!isActionExecuting && !currentlyExecutingAction.buttonPressTypeSettings.empty()) {
    currentlyExecutingAction.doAction();
    isActionExecuting = true;
    actionExecuteTime = millis();
  }
  if (isActionExecuting) {
    if (currentlyExecutingAction.canRepeatWhenHeld && isRepeatWhenHeld &&
        (millis() - actionExecuteTime) > currentlyExecutingAction.repeatPeriodMSec) {
      currentlyExecutingAction.doAction();
      actionExecuteTime = millis();
    }
    if (!MappedInputManager::isAnyPressed()) {
      currentlyExecutingAction = {};
      isActionExecuting = false;
      isRepeatWhenHeld = false;
    }
  }
}
