#pragma once

#include <map>
#include <vector>

#include "/util/ScreenshotUtil.h"
#include "GfxRenderer.h"
#include "MappedInputManager.h"
#include "activities/reader/EpubReaderActivity.h"

class ControlActionSettings {
 private:
  // Private constructor for singleton
  ControlActionSettings() = default;

  // Static instance
  static ControlActionSettings instance;

 public:
  // Delete copy constructor and assignment
  ControlActionSettings(const ControlActionSettings&) = delete;
  ControlActionSettings& operator=(const ControlActionSettings&) = delete;

  ~ControlActionSettings() = default;

  // Get singleton instance
  static ControlActionSettings& getInstance() { return instance; }

 public:
  // Actions that trigger in Reader Activities (i.e. Epub, Txt, and Xtc Reader Activities).
  std::map<MappedInputManager::ButtonAndPressType, StrId> ReaderButtonActionMap;
  std::map<MappedInputManager::ButtonAndPressType, StrId> ReaderTiltActionMap;

  // The default mapping between buttons with press type combination and StrId (action) in Reader Activities.
  std::map<MappedInputManager::ButtonAndPressType, StrId> ReaderDefaultButtonToStrIdMap = {
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Back},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_HOME_2},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Confirm},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_READER_MENU},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Left},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_PREV_PAGE_2},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Right},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_NEXT_PAGE_2},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Up},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_PREV_PAGE_2},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Down},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_NEXT_PAGE_2},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Back},
                                              MappedInputManager::BUTTON_PRESS_TYPE::LONG},
       StrId::STR_BROWSE_FILES},
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Confirm},
                                              MappedInputManager::BUTTON_PRESS_TYPE::LONG},
       StrId::STR_ADD_BOOKMARK}};

  // The default mapping between tilts with gesture type combination and StrId (action) in Reader Activities.
  std::map<MappedInputManager::TiltAndGestureType, StrId> ReaderDefaultTiltToStrIdMap = {
      {MappedInputManager::TiltAndGestureType{MappedInputManager::Tilt::TiltLeft,
                                              MappedInputManager::TILT_GESTURE_TYPE::SHORT},
       StrId::STR_PREV_PAGE_2},
      {MappedInputManager::TiltAndGestureType{MappedInputManager::Tilt::TiltRight,
                                              MappedInputManager::TILT_GESTURE_TYPE::SHORT},
       StrId::STR_NEXT_PAGE_2}};

  // The set of button with press type combination that is not allowed to be assigned by user in Reader Activities.
  std::set<MappedInputManager::ButtonAndPressType> ReaderReservedButtonPressType = {
      MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Back},
                                             MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
      MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Confirm},
                                             MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
      MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Left},
                                             MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
      MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Right},
                                             MappedInputManager::BUTTON_PRESS_TYPE::SHORT}};

  // The set of tilt with gesture type combination that is not allowed to be assigned by user for this activity.
  std::set<MappedInputManager::ButtonAndPressType> ReaderReservedTiltGestureType = {};

  std::set<StrId> ReaderUserAssignableActions{
      StrId::STR_NEXT_PAGE_2,  StrId::STR_PREV_PAGE_2, StrId::STR_NEXT_CHAPTER,
      StrId::STR_PREV_CHAPTER, StrId::STR_READER_MENU, StrId::STR_HOME_2,
      StrId::STR_ADD_BOOKMARK, StrId::STR_FOOTNOTES,   StrId::STR_LONG_PRESS_BEHAVIOR_ORIENTATION,
  };

  // Actions that trigger in any activity (e.g. sleep, take screenshot, and force screen refresh)
  std::map<MappedInputManager::ButtonAndPressType, StrId> GlobalButtonActionMap;
  std::map<MappedInputManager::ButtonAndPressType, StrId> GlobalTiltActionMap;

  // The default mapping between buttons with press type combination and StrId (action) in all Activities.

  std::map<MappedInputManager::ButtonAndPressType, StrId> GlobalDefaultButtonActionMap = {
      {MappedInputManager::ButtonAndPressType{{MappedInputManager::Button::Power, MappedInputManager::Button::Down},
                                              MappedInputManager::BUTTON_PRESS_TYPE::SHORT},
       StrId::STR_SCREENSHOT_BUTTON}};

  void readAndValidate(HalFile& file, uint8_t& member);
  bool saveToFile() const;
  bool loadFromFile();

  MappedInputManager::ButtonAndPressType getPowerButtonAndPressType();
};

// Helper macro to access control action settings
#define CONTROLACTIONSETTINGS ControlActionSettings::getInstance();