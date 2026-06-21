/**
 * Uses the Arduino Preferences library to store the following data;-
 *  - Node ID
 *  - Factory reset value (0 or 1)
 *  - Current WiFi SSID and password
 *  - ?? info re server pull configuration ??
 */
#include <Arduino.h>
#include <Preferences.h>
#include "NodeID.h"

#define NAMESPACE_NODEID "NodeID"
#define NAMESPACE_FACTORY_RESET "Reset"
#define NAMESPACE_WIFI "WiFi"

namespace ConfigurationPreferences {

  NodeID getNodeID(NodeID defaultNodeID);
  int getFactoryReset();
  const char*  getWiFiSSID();
  const char* getWiFiPassword();

  void putNodeID(NodeID nodeID);
  void putFactoryReset(int factoryReset);
  void putWiFiSSID(String wifiSSID);
  void putWiFiPassword(String wifiPassword);
    
}
