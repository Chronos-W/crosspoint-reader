#include <GfxRenderer.h>

#include "I18nKeys.h"
#include "MappedInputManager.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class ControlActionSettingsActivity final : public Activity {
 public:
  explicit ControlActionSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ControlActionSettings", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  ButtonNavigator buttonNavigator;

  int selectedIndex = 0;
  // Decided in onEnter() based on halTilt.isAvailable() so tilt gesture entries are hidden on X4.
  int visibleItemCount = 0;

  void handleSelection();

  enum MenuItem {
    BACK_BUTTON,
    CONFIRM_BUTTON,
    LEFT_BUTTON,
    RIGHT_BUTTON,
    UP_BUTTON,
    DOWN_BUTTON,
    POWER_BUTTON,
    LEFT_TILT,   // Gyro supported device
    RIGHT_TILT,  // Gyro supported device
    ITEM_COUNT
  };

  const StrId menuItemNames[ITEM_COUNT] = {
      StrId::STR_BACK_BUTTON,  StrId::STR_CONFIRM_BUTTON, StrId::STR_LEFT_BUTTON,
      StrId::STR_RIGHT_BUTTON, StrId::STR_UP_BUTTON,      StrId::STR_DOWN_BUTTON,
      StrId::STR_POWER_BUTTON, StrId::STR_LEFT_TILT,      StrId::STR_RIGHT_TILT,
  };
};