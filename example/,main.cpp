#include <Arduino.h>
#include "credentials.h"
#include "ESP32WiFiGC_V2.h"
#include "configurationOTA.h"
#include "configurationPreferences.h"

#define DEBUG Serial
#define NOCAN

// Board definitions
#define MANU "J Callingham"  // The manufacturer of node
#define MODEL "ESP32_2Servo_2Frog_2TOTI_Wifi" // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "1.0.1"   // Software version

// To Reset the Node Number, Uncomment and edit the next line
// Need to do this at least once.  
// #define NODE_ADDRESS  5,1,1,1,0x91,0x07  // First servo node.
// #define NODE_ADDRESS  5,1,1,1,0x91,0x08  // Second servo node.
#define NODE_ADDRESS  5,1,1,1,0x91,0x09  // Third servo node.

// Set to 1 to Force Reset EEPROM to Factory Defaults 
// Need to do this at least once.  
#define RESET_TO_FACTORY_DEFAULTS 0

/**
 * Various #includes from the OpenLCB_Single_Thread library.
 */
#include "mdebugging.h"           // debugging
#include "processor.h"
#include "processCAN.h"
#include "OpenLCBHeader.h"

/***
 * Much code removed
 */

// Was "NodeID nodeid(NODE_ADDRESS);" which was moved here for version 0.1.19.
// The actual value for Node ID is now set in setup() using data from Preferences or
// uses NODE_ADDRESS if not available in Preferences.
NodeID nodeid;

// The following #include needs nodeid to be already declared.
#include "OpenLCBMid.h"   // Essential - do not move or delete

// ==== Setup does initial configuration ======================
void setup() {
  Serial.begin(115200);

  // Delay to allow Serial port to be established.
  delay(1000);

  // temp for testing -- allows CoolTerm to be connected.
  delay(4000);

  Serial.printf("\n%6ld starting program", millis());
  Serial.printf("\n%6ld            Model: ", millis()); Serial.print(MODEL);
  Serial.printf("\n%6ld Software version: ", millis()); Serial.print(SWVERSION);
  Serial.printf("\n%6ld Compilation date: ", millis()); Serial.print(__DATE__);
  Serial.printf("\n%6ld Compilation time: ", millis()); Serial.print(__TIME__);

  // Create a ConfigurationOTA object and pass in the required parameters.
  ConfigurationOTA configurationOTA;
  configurationOTA.setCredentials(credentials); // A pointer to the credentials data in credentials.h
  configurationOTA.setTimeout(1000); // The 1000 mS timeout is used when connecting to one of potentially many WiFi hubs as not every WiFi hub may be available
  configurationOTA.setCurrentVersion(SWVERSION); // The currently running version of firmware
  configurationOTA.setDefaultNodeID(NodeID(NODE_ADDRESS)); // Used if a Node ID cannot be obtained

  // Connect to a WiFi hub, download the json configuration file and perform all configuration.
  configurationOTA.doConfiguration();

  // Update nodeid according to the Node ID stored in Preferences.
  // If there is no Node ID stored, then use the default of NODE_ADDRESS.
  nodeid = ConfigurationPreferences::getNodeID(NodeID(NODE_ADDRESS));

  // Initialise Olcb with the node id from Preferences.
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  Serial.printf("\n%6ld initialisation finished", millis());
}

// ==== Loop ==========================
void loop() {
  // Do OpenLCB/LCC processing.
  Olcb_process();

}

