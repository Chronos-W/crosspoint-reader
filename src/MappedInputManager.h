#pragma once

#include <HalGPIO.h>

class GfxRenderer;

#include <functional>
#include <map>
#include <set>
#include <utility>

#include "HalTiltSensor.h"
#include "I18nKeys.h"

class MappedInputManager {
 public:
  enum class Button { Back, Confirm, Left, Right, Up, Down, Power, PageBack, PageForward, NavNext, NavPrevious };

  // In the future, can consider adding TiltUp, TiltDown, RotateLeft, RotateRight.
  enum class Tilt { TiltLeft, TiltRight };

  struct Labels {
    const char* btn1;
    const char* btn2;
    const char* btn3;
    const char* btn4;
  };

  MappedInputManager(HalGPIO& gpio, const GfxRenderer& renderer) : gpio(gpio), renderer(renderer) {}
  // explicit MappedInputManager(HalGPIO& gpio, HalTiltSensor& tiltSensor) : gpio(gpio), tiltSensor(tiltSensor) {}

  void update() const { gpio.update(); }
  bool wasPressed(Button button) const;
  bool wasReleased(Button button) const;
  bool isPressed(Button button) const;
  bool isAnyPressed(Button button) const;
  bool wasAnyPressed() const;
  bool wasAnyReleased() const;
  unsigned long getHeldTime() const;
  Labels mapLabels(const char* back, const char* confirm, const char* previous, const char* next) const;

 public:
  // Get all buttons that is being pressed.
  std::set<Button> getButtonsIsPressed() const;

  // Get all buttons that was being pressed.
  std::set<Button> getButtonsWasPressed() const;

  // Get all buttons that was released.
  std::set<Button> getButtonsWasReleased() const;

  static constexpr uint16_t BUTTON_REPEAT_PERIOD_MSEC =
      500;  //  When button(s) are held and action already triggered once, amount of time that must pass before action
            //  will be triggered again.
  static constexpr uint16_t BUTTON_LONG_PRESS_DURATION_MSEC =
      500;  // For Long Press type, determines how long the button(s) must be held to trigger action.
  static constexpr uint16_t BUTTON_DOUBLE_PRESS_DURATION_MSEC =
      200;  // For Double Press type, determines how quickly the button(s) must be pressed twice to trigger action.

  // Need to figure out if debounce value HalTiltSensor::COOLDOWN_MS can be reduced, before this can be used.
  // static constexpr uint16_t TILT_DOUBLE_DURATION_MSEC = 400; // For Double Tilt type, determines how quickly the
  // device must be tilted twice to trigger action.

  enum class BUTTON_PRESS_TYPE { SHORT, LONG, DOUBLE, PRESS_TYPE_COUNT };
  enum class TILT_GESTURE_TYPE { SHORT, DOUBLE, PRESS_TYPE_COUNT };

  // When in an activity, these are the activation conditions that when fulfilled will uniquely identify the action that
  // is to be triggered. For example, Power button + Up button with Long Press to open a pop-up menu.
  struct ButtonAndPressType {
    std::set<Button> buttons;
    BUTTON_PRESS_TYPE buttonPressType;
    // auto operator<=>(const ButtonAndPressType&) const = default;
  };

  // When in an activity, these are the activation conditions that when fulfilled will uniquely identify the action that
  // is to be triggered. For example, Power button + Up button with Long Press to open a pop-up menu.
  struct TiltAndGestureType {
    Tilt tilt;
    TILT_GESTURE_TYPE tiltGestureType;
    // auto operator<=>(const ButtonAndPressType&) const = default;
  };

  // Get a map of buttons to actions (e.g. Power button may have a short press action, long press action, and double
  // press action).
  std::map<std::set<Button>, std::vector<StrId>> mapButtonsToActions(
      std::map<ButtonAndPressType, StrId> ButtonSettingsToActionMap) const;

  // When activation conditions for the control action are fulfilled, perform the control action. Only one control
  // action can be executing at any moment.
  StrId processInputButton(std::map<std::set<Button>, std::vector<StrId>> map) const;
  // Returns the raw front button index that was pressed this frame (or -1 if none).

  // Returns the raw button index that was pressed this frame (or -1 if none).
  int getPressedRawButton() const;

  // Workaround to handle global actions: sleep, take screenshot, force refresh.
  bool isSleepActionTriggered = false;
  bool isTakeScreenshotTriggered = false;
  bool isForceRefreshTriggered = false;

  // True when the control axis is flipped relative to the physical buttons: the user opted into
  // orientation-following front buttons AND the screen is *currently rendered* rotated (INVERTED /
  // LANDSCAPE_CCW). Keyed on the live renderer orientation rather than the persisted reader setting,
  // so portrait UI (home, settings) never swaps while the reader and its menus do.
  [[nodiscard]] bool isNavDirectionSwapped() const;

 private:
  HalGPIO& gpio;
  // Logical-to-physical button mapping depends on what the user is actually looking at: when the
  // screen is rendered rotated, the directional buttons must flip to match. The renderer is the only
  // authority on the *live* orientation (the reader rotates it and restores portrait on exit), so we
  // read it here instead of CrossPointSettings.orientation, which is just the persisted reader
  // preference and stays "rotated" even while portrait UI like home/settings is on screen.
  const GfxRenderer& renderer;

  bool mapButton(Button button, bool (HalGPIO::*fn)(uint8_t) const) const;
};
