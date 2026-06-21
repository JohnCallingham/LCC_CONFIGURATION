#include "configurationOTA.h"

int ConfigurationOTA::downloadConfiguration(const char* credentials) {
  // Connect to an SSID which has a "configuration_url" stored.

  Serial.printf("\n%6ld credentials=%s", millis(), credentials);

  // Deserialise the json credentials file.
  DeserializationError errorCredentials = deserializeJson(docCredentials, credentials);
  if (errorCredentials != DeserializationError::Ok) {
    Serial.printf("\nError deserialising credentials");
    return -1;
  }

  // Try all SSIDs in the credentials file which have a non zero "configuration_url" value.
  int error;
  for (JsonObject elemCredential : docCredentials.as<JsonArray>()) {
    if (! elemCredential["configuration_url"].isNull()) {
      error = processConfigurationCredential(elemCredential);
      if (error == 0) { break; } // No need to try any more SSIDs.
    }
  }

  // We've finished downloading the json configuration file.
  Serial.printf("\n%6ld Disconnecting from configuration WiFi: %s", millis(), WiFi.SSID());
  WiFi.disconnect(); 

  return 0;
}

int ConfigurationOTA::processConfigurationCredential(JsonObject elemCredential) {
  // Try this SSID to see if we can connect to it.
  if (connectWiFi(elemCredential["ssid"], elemCredential["password"]) != 0) {
    return -1;
  }

  // Successfully connected to this ssid.

  // Try this SSID to see if a configuration file can be downloaded.
  String payload = downloadJsonConfigurationFile(elemCredential["configuration_url"]);

  if (payload == "") {
    Serial.printf("\nUnable to download json configuration file");
    return -1;
  }

  // We have successfully downloaded the json configuration file.

  // Deserialise the json configuration file.
  DeserializationError errorConfigurations = deserializeJson(docConfigurations, payload.c_str());
  if (errorConfigurations != DeserializationError::Ok) {
    Serial.printf("\nError deserialising configuration");
    return -1; // Try other SSIDs.
  }
  Serial.printf("\n%6ld Deserialised json configuration file", millis());

  // Step through all configurations looking for one which matches our MAC address.
  for (JsonObject elemConfiguration : docConfigurations["Configurations"].as<JsonArray>()) {
    processConfiguration(elemConfiguration);
  }

  return 0;
}

void ConfigurationOTA::processConfiguration(JsonObject elemConfiguration) {
  if (! (elemConfiguration["MAC_Address"] == macAddress)) { // != doesn't work !!!
    return;
  }

  // This is the configuration record for this node.
  Serial.printf("\n%6ld Found matching MAC address", millis());

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
      processJMRICredential(elemCredential);
      break; // No need to try any other credential records.
    }
  }
}

void ConfigurationOTA::processJMRICredential(JsonObject elemCredential) {
  strncpy(configurationJMRIssid, elemCredential["ssid"], sizeof(configurationJMRIssid));
  strncpy(configurationJMRIpassword, elemCredential["password"], sizeof(configurationJMRIpassword));

  // === ??? need to store ssid and passowrd here if they have changed ??? ===
  // === if ConnectionPreferences was ?static? it could be used here without an object ===


  // the original connect to jmri code to use the ssid and password from preferences

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
  Serial.printf("\n%6ld Downloading jsonURL: %s", millis(), jsonURL.c_str());

  String Payload;
  int httpResponseCode = downloadJson(jsonURL.c_str(), Payload);
  if (httpResponseCode != 200)
      return "";
  
  Serial.printf("\n%6ld Payload=\n%s", millis(), Payload.c_str());

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




// int ConfigurationOTA::getConfiguration(const char* credentials) {
//   // Connect to an SSID which has a "configuration_url" stored.

//   Serial.printf("\ncredentials=%s", credentials);

//   // Deserialise the json credentials file.
//   JsonDocument docCredentials;
//   DeserializationError errorCredentials = deserializeJson(docCredentials, credentials);
//   if (errorCredentials != DeserializationError::Ok) {
//     Serial.printf("\nError deserialising credentials");
//     return -1;
//   }

//   // Try all SSIDs in the credentials file.
//   for (JsonObject elemCredential : docCredentials.as<JsonArray>()) {
//     // Does this SSID have a non zero "configuration_url" value?
//     if (! elemCredential["configuration_url"].isNull()) {

//       // Try this SSID to see if we can connect to it.
//       if (connectWiFi(elemCredential["ssid"], elemCredential["password"]) == 0) {
//         // Successfully connected to this ssid.

//         // Try this SSID to see if a configuration file can be downloaded.
//         String payload = downloadJsonConfigurationFile(elemCredential["configuration_url"]);

//         if (payload == "") {
//           Serial.printf("\nUnable to download json configuration file");
//         } else {
//           // We have successfully downloaded the json configuration file.

//           // Deserialise the json configuration file.
//           JsonDocument docConfigurations;
//           DeserializationError errorConfigurations = deserializeJson(docConfigurations, payload.c_str());
//           if (errorConfigurations != DeserializationError::Ok) {
//             Serial.printf("\nError deserialising configuration");
//             continue; // Try other SSIDs.
//           }
//           Serial.printf("\nDeserialised json configuration file");

//           // Step through all configurations looking for one which matches our MAC address.
//           for (JsonObject elemConfiguration : docConfigurations["Configurations"].as<JsonArray>()) {
//             if (elemConfiguration["MAC_Address"] == macAddress) {
//               // This is the configuration record for this node.
//               Serial.printf("\nFound matching MAC address");

//               // Copy so the data is not lost when the JsonObject goes out of scope.
//               strncpy(configurationBoard, elemConfiguration["Board"], sizeof(configurationBoard));
//               strncpy(configurationNodeID, elemConfiguration["NodeID"], sizeof(configurationNodeID));
//               strncpy(configurationVersion, elemConfiguration["Version"], sizeof(configurationVersion));
//               strncpy(configurationUpdateURL, elemConfiguration["UpdateURL"], sizeof(configurationUpdateURL));
//               strncpy(configurationJMRIname, elemConfiguration["JMRI_name"], sizeof(configurationJMRIname));

//               // Look up the SSID and Password from credentials.h for JMRI_name.
//               // Step through all credential records looking for one which matches JMRI_name.
//               for (JsonObject elemCredential : docCredentials.as<JsonArray>()) {
//                 if (strcmp(elemCredential["name"], configurationJMRIname) == 0) {
//                   strncpy(configurationJMRIssid, elemCredential["ssid"], sizeof(configurationJMRIssid));
//                   strncpy(configurationJMRIpassword, elemCredential["password"], sizeof(configurationJMRIpassword));

//                   break; // No need to try any other credential records.
//                 }
//               }
//             }
//           }
//           break; // No need to try any other SSIDs.
//         }
//       }
//     }
//   }

//   // We've finished downloading the json configuration file.
//   Serial.printf("\nDisconnecting from configuration WiFi");
//   WiFi.disconnect(); 

//   return 0;
// }
