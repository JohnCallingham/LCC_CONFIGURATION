#ifndef CONFIGURATION_MENU_H
#define CONFIGURATION_MENU_H

#include <Preferences.h>
#include "NodeID.h"

class ConfigurationMenu {
  public:
    ConfigurationMenu(NodeID defaultNodeID, uint8_t defaultFactoryReset);
    void showMenu();
    NodeID getNodeID();
    uint8_t getFactoryReset();

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
};

#endif
