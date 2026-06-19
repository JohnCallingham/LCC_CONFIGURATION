#include "configurationOTA.h"

int ConfigurationOTA::getConfiguration(const char* credentials) {
  // Connect to an SSID which has a "configuration_url" stored.

  Serial.printf("\ncredentials=%s", credentials);

  // Deserialise the json credentials file.
  JsonDocument docCredentials;
  DeserializationError errorCredentials = deserializeJson(docCredentials, credentials);
  if (errorCredentials != DeserializationError::Ok) {
    Serial.printf("\nError deserialising credentials");
    return -1;
  }

  // Try all SSIDs in the credentials file.
  for (JsonObject elemCredential : docCredentials.as<JsonArray>()) {
    // Does this SSID have a non zero "configuration_url" value?
    if (! elemCredential["configuration_url"].isNull()) {

      // Try this SSID to see if we can connect to it.
      if (connectWiFi(elemCredential["ssid"], elemCredential["password"]) == 0) {
        // Successfully connected to this ssid.

        // Try this SSID to see if a configuration file can be downloaded.
        String payload = downloadJsonConfigurationFile(elemCredential["configuration_url"]);

        if (payload == "") {
          Serial.printf("\nUnable to download json configuration file");
        } else {
          // We have successfully downloaded the json configuration file.

          // Deserialise the json configuration file.
          JsonDocument docConfigurations;
          DeserializationError errorConfigurations = deserializeJson(docConfigurations, payload.c_str());
          if (errorConfigurations != DeserializationError::Ok) {
            Serial.printf("\nError deserialising configuration");
            continue; // Try other SSIDs.
          }
          Serial.printf("\nDeserialised json configuration file");

          // Step through all configurations looking for one which matches our MAC address.
          for (JsonObject elemConfiguration : docConfigurations["Configurations"].as<JsonArray>()) {
            if (elemConfiguration["MAC_Address"] == macAddress) {
              // This is the configuration record for this node.
              Serial.printf("\nFound matching MAC address");

              // Copy so the data is not lost when the JsonObject goes out of scope.
              strncpy(configurationBoard, elemConfiguration["Board"], sizeof(configurationBoard));
              strncpy(configurationNodeID, elemConfiguration["NodeID"], sizeof(configurationNodeID));
              strncpy(configurationVersion, elemConfiguration["Version"], sizeof(configurationVersion));
              strncpy(configurationUpdateURL, elemConfiguration["UpdateURL"], sizeof(configurationUpdateURL));
              strncpy(configurationJMRIname, elemConfiguration["JMRI_name"], sizeof(configurationJMRIname));

              // Look up the SSID and Password from credentials.h for JMRI_name.
              // Step through all credential records looking for one which matches JMRI_name.
              for (JsonObject elemCredential : docCredentials.as<JsonArray>()) {
                if (strcmp(elemCredential["name"], configurationJMRIname) == 0) {
                  strncpy(configurationJMRIssid, elemCredential["ssid"], sizeof(configurationJMRIssid));
                  strncpy(configurationJMRIpassword, elemCredential["password"], sizeof(configurationJMRIpassword));

                  break; // No need to try any other credential records.
                }
              }
            }
          }
          break; // No need to try any other SSIDs.
        }
      }
    }
  }

  // We've finished downloading the json configuration file.
  Serial.printf("\nDisconnecting from configuration WiFi");
  WiFi.disconnect(); 

  return 0;
}

int ConfigurationOTA::connectWiFi(String ssid, String password) {
  // Timeout if no response.
  long timeoutTime = millis() + ssidTimeoutmS;

  Serial.printf("\nConnecting to %s ", ssid.c_str());

  WiFi.begin(ssid, password);
	while ((! WiFi.isConnected()) && (millis() < timeoutTime)) {
		Serial.print(".");
		delay(100);
	}

  if (! WiFi.isConnected()) {
    Serial.printf("\nTimed out connecting to %s", ssid.c_str());
    return -1;
  }

  macAddress = WiFi.macAddress();

  Serial.printf("\nConnected to %s", ssid.c_str());
  Serial.printf("\nMAC address = %s", macAddress.c_str());

  return 0;
}

String ConfigurationOTA::downloadJsonConfigurationFile(String jsonURL) {
  // Returns the JSON text as a String.
  Serial.printf("\nDownloading jsonURL: %s", jsonURL.c_str());

  String Payload;
  int httpResponseCode = downloadJson(jsonURL.c_str(), Payload);
  if (httpResponseCode != 200)
      return "";
  
  Serial.printf("\nPayload=\n%s", Payload.c_str());

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

// NodeID ConfigurationOTA::getConfigurationNodeIDAsNodeID() {
NodeID ConfigurationOTA::nodeID() {
  // Parses the char configurationNodeID[20] to return a NodeID object.
  // Assumes that configurationNodeID is in the foramt "xx.xx.xx.xx.xx.xx" where xx are two hext digits.

  // Serial.printf("\nNode ID = %s", configurationNodeID);

  char cval[3]; // char value
  int ival[6]; // integer values

  cval[2] = '\0';

  for (int i = 0; i < 6; i++) {
    strncpy(cval, configurationNodeID + (i * 3), 2);
    ival[i] = strtol(cval, NULL, 16);
    // Serial.printf("\nval[%d] = %d", i, ival[i]);
  }

  return NodeID(ival[0], ival[1], ival[2], ival[3], ival[4], ival[5]);
}
