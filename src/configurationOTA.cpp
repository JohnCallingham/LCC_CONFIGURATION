#include "configurationOTA.h"
#include "configurationPreferences.h"

void ConfigurationOTA::doConfiguration() {
  int error = downloadConfiguration();

  if (error == 0) {
    // Configuration data was successfully downloaded.

    // If a newer firmware is available, then use it to update the existing firmware and restart.
    checkForFirmwareUpdate(this->currentVersion);

    // Compare the Node ID in the configuration data with the Node ID stored in Preferences.
    // If they are different, then store the new Node ID in Preferences.
    // NodeID preferencesNodeID = ConfigurationPreferences::getNodeID(NodeID(NODE_ADDRESS));
    NodeID preferencesNodeID = ConfigurationPreferences::getNodeID(defaultNodeID);
    if (! this->nodeID().equals(& preferencesNodeID)) {
      // Store the new Node ID in Preferences.
      ConfigurationPreferences::putNodeID(this->nodeID());
    }
  }

  // We've finished performing configuration.
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n%6ld Disconnecting from configuration WiFi: %s", millis(), WiFi.SSID());
    WiFi.disconnect(); 
  }
}

// int ConfigurationOTA::downloadConfiguration(const char* credentials, long ssidTimeoutmS) {
int ConfigurationOTA::downloadConfiguration() {
  // Connect to an SSID which has a "configuration_url" stored.

  Serial.printf("\n%6ld Contents of credentials.h;-%s", millis(), this->credentials);

  // Deserialise the json credentials file.
  DeserializationError errorCredentials = deserializeJson(docCredentials, this->credentials);
  if (errorCredentials != DeserializationError::Ok) {
    Serial.printf("\n%6ld Error deserialising credentials", millis());
    return -1;
  }
  Serial.printf("\n%6ld Deserialised credentials file", millis());

  // Try all SSIDs in the credentials file which have a non zero "configuration_url" value.
  int error = -1;
  for (JsonObject elemCredential : docCredentials["Credentials"].as<JsonArray>()) {
    if (! elemCredential["configuration_url"].isNull()) {
      error = processConfigurationCredential(elemCredential);
      if (error == 0) { break; } // No need to try any more SSIDs.
    }
  }

  if (error == -1) {
    Serial.printf("\n%6ld No available SSIDs", millis());
  }

  if (error != 0) {
    Serial.printf("\n%6ld  Error when attempting to download a json configuration file", millis());
  }

  Serial.printf("\n%6ld Exiting downloadConfiguration()", millis());
  return error;
}

int ConfigurationOTA::processConfigurationCredential(JsonObject elemCredential) {
  Serial.printf("\n%6ld In processConfigurationCredential()", millis());

  // Try this SSID to see if we can connect to it.
  if (connectWiFi(elemCredential["ssid"], elemCredential["password"]) != 0) {
    return -1;
  }

  // Successfully connected to this ssid.

  // Try this SSID to see if a configuration file can be downloaded.
  String payload = downloadJsonConfigurationFile(elemCredential["configuration_url"]);

  if (payload == "") {
    Serial.printf("\n%6ld Unable to download json configuration file", millis());
    return -1;
  }

  // We have successfully downloaded the json configuration file.
  Serial.printf("\n%6ld Downloaded json configuration file", millis());

  // Deserialise the json configuration file.
  DeserializationError errorConfigurations = deserializeJson(docConfigurations, payload.c_str());
  if (errorConfigurations != DeserializationError::Ok) {
    Serial.printf("\n%6ld  Error deserialising configuration", millis());
    return -1; // Try other SSIDs.
  }
  Serial.printf("\n%6ld Deserialised json configuration file", millis());

  // Step through all configurations looking for one which matches our MAC address.
  int error = -1;
  for (JsonObject elemConfiguration : docConfigurations["Configurations"].as<JsonArray>()) {
    error = processConfiguration(elemConfiguration);
    if (error == 0) { break; } // We have found a matching MAC address so no need to continue round the for loop.
  }

  Serial.printf("\n%6ld Exiting processConfigurationCredential()", millis());
  return error; // Returns -1 if no matching MAC address found.
}

int ConfigurationOTA::processConfiguration(JsonObject elemConfiguration) {
  Serial.printf("\n%6ld In processConfiguration()", millis());

  if (! (elemConfiguration["MAC_Address"] == macAddress)) { // != doesn't work !!
    Serial.printf("\n%6ld  Non matching MAC address: %s", millis(), elemConfiguration["MAC_Address"].as<const char *>());
    return -1;
  }

  // This is the configuration record for this node.
  Serial.printf("\n%6ld  Found matching MAC address: %s", millis(), elemConfiguration["MAC_Address"].as<const char *>());

  // Copy so the data is not lost when the JsonObject goes out of scope.
  strncpy(configurationBoard, elemConfiguration["Board"], sizeof(configurationBoard));
  strncpy(configurationNodeID, elemConfiguration["NodeID"], sizeof(configurationNodeID));
  strncpy(configurationUpdatePath, elemConfiguration["Update"]["Path"], sizeof(configurationUpdatePath));
  strncpy(configurationUpdateVersion, elemConfiguration["Update"]["Version"], sizeof(configurationUpdateVersion));
  strncpy(configurationUpdateFilename, elemConfiguration["Update"]["Filename"], sizeof(configurationUpdateFilename));
  strncpy(configurationJMRIname, elemConfiguration["JMRI_name"], sizeof(configurationJMRIname));

  Serial.printf("\n%6ld  Board = %s", millis(), this->board());
  Serial.printf("\n%6ld  NodeID = %s", millis(), this->printNodeID(this->nodeID()));
  Serial.printf("\n%6ld  UpdatePath = %s", millis(), this->updatePath());
  Serial.printf("\n%6ld  UpdateVersion = %s", millis(), this->updateVersion());
  Serial.printf("\n%6ld  UpdateFilename = %s", millis(), this->updateFilename());
  Serial.printf("\n%6ld  JMRIname = %s", millis(), this->jmriName());

  // Look up the SSID and Password from credentials.h for JMRI_name.
  // Step through all credential records looking for one which matches JMRI_name.
  for (JsonObject elemCredential : docCredentials["Credentials"].as<JsonArray>()) {
    if (strcmp(elemCredential["name"], configurationJMRIname) == 0) {
      processJMRICredential(elemCredential);
      break; // No need to try any other credential records.
    }
  }

  Serial.printf("\n%6ld Exiting processConfiguration()", millis());
  return 0;
}

void ConfigurationOTA::processJMRICredential(JsonObject elemCredential) {
  Serial.printf("\n%6ld In processJMRICredential()", millis());

  strncpy(configurationJMRIssid, elemCredential["ssid"], sizeof(configurationJMRIssid));
  strncpy(configurationJMRIpassword, elemCredential["password"], sizeof(configurationJMRIpassword));

  Serial.printf("\n%6ld  configurationJMRIssid = %s", millis(), configurationJMRIssid);
  Serial.printf("\n%6ld  ConfigurationPreferences::getWiFiSSID() = %s", millis(), ConfigurationPreferences::getWiFiSSID());

  // If either of ssid or password has changed from that stored in Preferences, update Preferences.
  if ((strcmp(configurationJMRIssid, ConfigurationPreferences::getWiFiSSID()) != 0) ||
      (strcmp(configurationJMRIpassword, ConfigurationPreferences::getWiFiPassword()) != 0)) {
    // ssid or password has changed

    // String strWiFissid = String(configurationJMRIssid);
    // String strWiFipassword = String(configurationJMRIpassword);

    Serial.printf("\n%6ld  Storing new ssid: %s", millis(), configurationJMRIssid);

    // ConfigurationPreferences::putWiFiSSID(strWiFissid);
    // ConfigurationPreferences::putWiFiPassword(strWiFipassword);
    // === NOT been tested !!! ===  
    ConfigurationPreferences::putWiFiSSID(configurationJMRIssid);
    ConfigurationPreferences::putWiFiPassword(configurationJMRIpassword);
  }
  Serial.printf("\n%6ld Exiting processJMRICredential()", millis());
}

void ConfigurationOTA::checkForFirmwareUpdate(String swVersion) {
  // Called from main.cpp

  Serial.printf("\n%6ld In checkForFirmwareUpdate()", millis());
  Serial.printf("\n%6ld  installed version is %s, configuration version is %s", millis(), swVersion.c_str(), configurationUpdateVersion);

  // Check that Update_Path, Update_Version and Update_Filename have been successfully downloaded.
  if ((strlen(configurationUpdatePath) == 0) || (strlen(configurationUpdateVersion) == 0) || (strlen(configurationUpdateFilename) == 0)) {
    Serial.printf("\n%6ld  Any of Update_Path, Update_Version or Update_Filename are not present", millis());
    return;
  }

  // Check to see if the installed version is different to [Update][Version].
  if (strcmp(swVersion.c_str(), configurationUpdateVersion) == 0) {
    // Versions are the same so nothing more to do.
    Serial.printf("\n%6ld Exiting checkForFirmwareUpdate()", millis());
    return;
  }

  // There is a new version to download, so calculate the full URL.
  char updateURL[250];
  sprintf(updateURL, "%s/V%s/%s", configurationUpdatePath, configurationUpdateVersion, configurationUpdateFilename);

  Serial.printf("\n%6ld  Starting firmware update", millis());

  int error = doFirmwareUpdate(updateURL);

  Serial.printf("\n%6ld Exiting checkForFirmwareUpdate()", millis());
}

int ConfigurationOTA::doFirmwareUpdate(const char* updateURL) {
  Serial.printf("\n%6ld In doFirmwareUpdate()", millis());
  Serial.printf("\n%6ld  updateURL = %s", millis(), updateURL);

  HTTPClient http;

  http.begin(updateURL);
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode != 200) {
    Serial.printf("\n%6ld httpResponseCode = %d", millis(), httpResponseCode);
    return -1;
  }

  int totalLength = http.getSize();

  Serial.printf("\n%6ld totalLength = %d ", millis(), totalLength);

  // this is required to start firmware update process
  if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
    Serial.printf("\n%6ld  Error from Update.begin()", millis());
    return -1;
  }

  // create buffer for read
  uint8_t buff[1280] = { 0 };

  // get tcp stream
  WiFiClient* stream = http.getStreamPtr();

  // read all data from server
  long timeForNextPrint = millis() + 1000;
  int offset = 0;
  while (http.connected() && offset < totalLength) {
    size_t sizeAvail = stream->available();
    if (sizeAvail > 0) {
      size_t bytes_to_read = min(sizeAvail, sizeof(buff));
      size_t bytes_read = stream->readBytes(buff, bytes_to_read);
      size_t bytes_written = Update.write(buff, bytes_read);
      if (bytes_read != bytes_written) {
        Serial.printf("\n%6ld Unexpected error in OTA: %d %d %d\n", millis(), bytes_to_read, bytes_read, bytes_written);
        break;
      }
      offset += bytes_written;

      // Print progress every second;
      if (millis() > timeForNextPrint) {
        Serial.printf("\n%6ld downloaded %d", millis(), offset);
        timeForNextPrint = millis() + 1000;
      }
    }
  }

  if (offset == totalLength) {
    Serial.printf("\n%6ld downloaded %d", millis(), offset);

    Update.end(true);

    Serial.printf("\n%6ld Restarting to load new firmware\n", millis());

    delay(1000);

    // Restart ESP32 to see changes
    ESP.restart();
  }

  http.end();
  return httpResponseCode;
}

int ConfigurationOTA::connectWiFi(String ssid, String password) {
  // Timeout if no response.
  long timeoutTime = millis() + ssidTimeoutmS;

  Serial.printf("\n%6ld Connecting to %s ", millis(), ssid.c_str());

  WiFi.begin(ssid, password);
	while ((! WiFi.isConnected()) && (millis() < timeoutTime)) {
		Serial.print(".");
		delay(100);
	}

  if (! WiFi.isConnected()) {
    Serial.printf("\n%6ld Timed out connecting to %s", millis(), ssid.c_str());
    return -1;
  }

  macAddress = WiFi.macAddress();

  Serial.printf("\n%6ld Connected to %s", millis(), ssid.c_str());
  Serial.printf("\n%6ld MAC address = %s", millis(), macAddress.c_str());

  return 0;
}

String ConfigurationOTA::downloadJsonConfigurationFile(String jsonURL) {
  // Returns the JSON text as a String.
  Serial.printf("\n%6ld Downloading configuration_url: %s", millis(), jsonURL.c_str());

  String Payload;
  int httpResponseCode = downloadJson(jsonURL.c_str(), Payload);
  if (httpResponseCode != 200)
      return "";
  
  Serial.printf("\n%6ld Contents of configuration_url;-\n%s", millis(), Payload.c_str());

  return Payload;
}

int ConfigurationOTA::downloadJson(const char* URL, String& payload)
{
    HTTPClient http;
    http.begin(URL);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        payload = http.getString();
    }

    // Free resources
    http.end();
    return httpResponseCode;
}

NodeID ConfigurationOTA::nodeID() {
  // Parses the char configurationNodeID[20] to return a NodeID object.
  // Assumes that configurationNodeID is in the foramt "xx.xx.xx.xx.xx.xx" where xx are two hext digits.
  char cval[3]; // char value
  int ival[6]; // integer values

  cval[2] = '\0';

  for (int i = 0; i < 6; i++) {
    strncpy(cval, configurationNodeID + (i * 3), 2);
    ival[i] = strtol(cval, NULL, 16);
  }

  return NodeID(ival[0], ival[1], ival[2], ival[3], ival[4], ival[5]);
}

const char* ConfigurationOTA::printNodeID(NodeID nodeID) {
  // Returns a printable string showing nodeID.
  sprintf(this->charNodeID, "%02X.%02X.%02X.%02X.%02X.%02X", nodeID.val[0], nodeID.val[1], nodeID.val[2], nodeID.val[3], nodeID.val[4], nodeID.val[5]);
  return this->charNodeID;
}
