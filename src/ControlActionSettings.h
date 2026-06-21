#pragma once

#include <map>
#include <vector>

#include "/util/ScreenshotUtil.h"
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
  std::map<MappedInputManager::ButtonAndPressType, StrId> ReaderButtonActionMapping;
  std::map<MappedInputManager::ButtonAndPressType, StrId> ReaderTiltActionMapping;

  // Actions that trigger in any activity (e.g. Sleep and Take Screenshot)
  std::map<MappedInputManager::ButtonAndPressType, StrId> GlobalButtonActionMapping;
  bool saveToFile() const;
  bool loadFromFile();

  void doGlobalAction(StrId actionName);

  // Need a place to handle global actions. There's probably a better place/way to do it but
  // this should be good enough for the moment.
  GfxRenderer& renderer;
  unsigned long allowSleepAt = 0;

  void getAllowSleepAt(unsigned long sleepAt);
  void getRenderer(GfxRenderer& renderer);

  MappedInputManager::ButtonAndPressType getPowerButtonAndPressType();
  void takeScreenshotControlAction(GfxRenderer& renderer);
  void sleepControlAction();
  void forceRefreshControlAction();
};

// Helper macro to access settings
#define CONTROLACTIONSETTINGS ControlActionSettings::getInstance();