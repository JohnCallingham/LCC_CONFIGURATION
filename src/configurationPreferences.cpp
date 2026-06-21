#include "configurationPreferences.h"

namespace ConfigurationPreferences {
  Preferences preferences;

  NodeID getNodeID(NodeID defaultNodeID) {
    // Does the namespace NAMESPACE_NODEID exist?
    preferences.begin(NAMESPACE_NODEID, false);
    bool doesExist = preferences.isKey("ID0");
    if (doesExist == false) {
      // The NodeID preferences namespace does not exist.
      return defaultNodeID;
    }

    preferences.begin(NAMESPACE_NODEID, true);
    uint8_t ID0 = preferences.getUChar("ID0");
    uint8_t ID1 = preferences.getUChar("ID1");
    uint8_t ID2 = preferences.getUChar("ID2");
    uint8_t ID3 = preferences.getUChar("ID3");
    uint8_t ID4 = preferences.getUChar("ID4");
    uint8_t ID5 = preferences.getUChar("ID5");
    preferences.end();

    return NodeID(ID0, ID1, ID2, ID3, ID4, ID5);
  }

  int getFactoryReset() {
    if (! preferences.begin(NAMESPACE_FACTORY_RESET, true)) {
      // The factory reset preferences namespace does not exist.
      return 0;
    }

    preferences.begin(NAMESPACE_FACTORY_RESET, true);
    int factoryReset = preferences.getUChar("Reset");
    preferences.end();

    return factoryReset;
  }

  String getWiFiSSID() {
    if (! preferences.begin(NAMESPACE_WIFI, true)) {
      // The WiFi preferences namespace does not exist.
      return "";
    }

    preferences.begin(NAMESPACE_WIFI, true);
    String ssid = preferences.getString("SSID");
    preferences.end();

    return ssid;
  }

  String getWiFiPassword() {
    if (! preferences.begin(NAMESPACE_WIFI, true)) {
      // The WiFi preferences namespace does not exist.
      return "";
    }

    preferences.begin(NAMESPACE_WIFI, true);
    String password = preferences.getString("password");
    preferences.end();

    return password;
  }

  void putNodeID(NodeID nodeID) {
    preferences.begin(NAMESPACE_NODEID);
    preferences.putUChar("ID0", nodeID.val[0]);
    preferences.putUChar("ID1", nodeID.val[1]);
    preferences.putUChar("ID2", nodeID.val[2]);
    preferences.putUChar("ID3", nodeID.val[3]);
    preferences.putUChar("ID4", nodeID.val[4]);
    preferences.putUChar("ID5", nodeID.val[5]);
    preferences.end();
    Serial.printf("\n%6ld putNodeID: written new node id: ", millis());
    nodeID.print();
  }

  void putFactoryReset(int factoryReset) {
    preferences.begin(NAMESPACE_FACTORY_RESET);
    preferences.putUChar("Reset", factoryReset);
    preferences.end();
  }

  void putWiFiSSID(String wifiSSID) {
    preferences.begin(NAMESPACE_WIFI);
    preferences.putString("SSID", wifiSSID);
    preferences.end();
  }

  void putWiFiPassword(String wifiPassword) {
    preferences.begin(NAMESPACE_WIFI);
    preferences.putString("password", wifiPassword);
    preferences.end();
  }
}
