#include "ControlActionSettings.h"

#include <CrossPointSettings.h>
#include <GfxRenderer.h>

#include "JsonSettingsIO.h"
#include "MappedInputManager.h"

// Initialize the static instance
ControlActionSettings ControlActionSettings::instance;

namespace {
constexpr uint8_t CONTROLACTIONSETTINGS_FILE_VERSION = 1;
constexpr char CONTROLACTIONSETTINGS_FILE_JSON[] = "/.crosspoint/controlactionsettings.json";
}  // namespace

void ControlActionSettings::readAndValidate(HalFile& file, uint8_t& member) {}

bool ControlActionSettings::saveToFile() const {
  return JsonSettingsIO::saveControlActionSettings(*this, CONTROLACTIONSETTINGS_FILE_JSON);
}

bool ControlActionSettings::loadFromFile() {
  // Try JSON first
  if (Storage.exists(CONTROLACTIONSETTINGS_FILE_JSON)) {
    String json = Storage.readFile(CONTROLACTIONSETTINGS_FILE_JSON);
    if (!json.isEmpty()) {
      bool resave = false;
      bool result = JsonSettingsIO::loadControlActionSettings(*this, json.c_str(), &resave);
      if (result && resave) {
        if (saveToFile()) {
          LOG_DBG("CPS", "Resaved control action settings to update format");
        } else {
          LOG_ERR("CPS", "Failed to resave control action settings after format update");
        }
      }
      return result;
    }
  }
  return false;
}