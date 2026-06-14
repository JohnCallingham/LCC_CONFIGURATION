#ifndef CONFIGURATION_MENU_H
#define CONFIGURATION_MENU_H

#include <Preferences.h>
#include "NodeID.h"
#include "ArduinoJson.h"

/**
 * Puzzle: In the file ESP32WiFiGC.h it doesn't matter whether or not
 * credentials.h is #included, or const char* credentials is declared to be extern.
 * BUT, in this file #include credentials.h causes an error and
 * const char* credentials has to be declared to be extern.
 */
//#include "credentials.h"
extern const char* credentials;

class ConfigurationMenu {
  public:
    ConfigurationMenu(NodeID defaultNodeID, uint8_t defaultFactoryReset);
    ConfigurationMenu(){}
    void showMenu();
    NodeID getNodeID();
    uint8_t getFactoryReset();
    String getSSID();
    String getPassword();

  private:
    int waitForEnterOrTimeout(long timeout);
    int waitForKeyPressed();
    String getStringUntilEnter();
    void doRestart();
    void doConfigureNodeID();
    void doConfigureWiFi();
    void doResetFactoryDefaults();

    Preferences preferences;

    NodeID nodeID; // The default node id.
    uint8_t factoryResetRequired; // The default value (0 or 1). 1 means do a factory reset at the next restart.

    // Used to deserialise the credentials JSON.
    JsonDocument doc;

};

#endif
