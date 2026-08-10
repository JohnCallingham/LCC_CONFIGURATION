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
#include <WiFi.h>

#define NAMESPACE_NODEID "NodeID"
#define NAMESPACE_FACTORY_RESET "Reset"
#define NAMESPACE_WIFI "WiFi"
#define NAMESPACE_HUB_IP_ADDRESS "HubIP"
#define NAMESPACE_NODE_IP_ADDRESS "NodeIP"

namespace ConfigurationPreferences {

  NodeID getNodeID(NodeID defaultNodeID);
  int getFactoryReset();
  const char*  getWiFiSSID();
  const char* getWiFiPassword();
  String getHubIPAddress();
  String getNodeIPAddress();

  void putNodeID(NodeID nodeID);
  void putFactoryReset(int factoryReset);
  void putWiFiSSID(String wifiSSID);
  void putWiFiSSID(const char* wifiSSID);
  void putWiFiPassword(String wifiPassword);
  void putWiFiPassword(const char* wifiPassword);
  void putHubIPAddress(String IP);
  void putNodeIPAddress(String IP);
    
}
