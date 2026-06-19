#ifndef CONFIGURATION_OTA_H
#define CONFIGURATION_OTA_H

/**
 * Allows the user to configure all nodes with one file stored externally to the nodes.
 * The file is in json format and stored on a web server on the local network.
 * On startup each node does;-
 *  1. Uses the json in credentials.h to connect to an SSID which has configuration data. May need to try multiple ones.
 *    a. Use the credentials json formatted list of SSIDs to test each one in turn.
 *    b. Only try those which have a "configuration_url" value stored.
 *  2. Connect to a configuration web server.
 *  3. Download the configuration JSON file from the web server.
 *  4. Deserialise the JSON configuration file.
 *  5. Determine which record in the configuration file matches this node's MAC address.
 *  6. Find the Node ID for this node's MAC address.
 *  7. If the Node ID in the configuration file is different to the Node ID in use, then update Preferences.
 *  8. The node will use the new Node ID.
 *  9. If the user wishes that all the node's events IDs are recalculated to reflect the new Node ID, then
 *       the user should select the 'Clear Everything back to Factory Defaults' option in JMRI's CLI editor
 *       before restarting the node.
 * 10. Poss also keep a record of the software installed on each node and its version.
 */

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
//#include <Update.h>
#include <WiFi.h>
#include "NodeID.h"

class ConfigurationOTA {
  public:
    ConfigurationOTA(long ssidTimeoutmS) { this->ssidTimeoutmS = ssidTimeoutmS; }
    int getConfiguration(const char* credentials);
    const char* board() { return configurationBoard; }
    NodeID nodeID();
    const char* version() { return configurationVersion; }
    const char* updateURL() { return configurationUpdateURL; }
    const char* jmriSSID() { return configurationJMRIssid; }
    const char* jmriPassword() { return configurationJMRIpassword; }

  private:
    int connectWiFi(String ssid, String password);
    String downloadJsonConfigurationFile(String jsonURL);
    int downloadJson(const char* URL, String& payload);

    long ssidTimeoutmS;

    String macAddress;
    
    // The values from the json configuration file.
    char configurationBoard[100] = "";
    char configurationNodeID[20] = "";
    char configurationVersion[20] = "";
    char configurationUpdateURL[200] = "";
    char configurationJMRIname[50] = "";
    char configurationJMRIssid[50] = "";
    char configurationJMRIpassword[50] = "";
};

#endif
