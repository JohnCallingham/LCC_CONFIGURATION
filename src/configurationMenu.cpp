#include "configurationMenu.h"

ConfigurationMenu::ConfigurationMenu(NodeID defaultNodeID, uint8_t defaultFactoryReset) {
    // Store the default values.
    this->nodeID = defaultNodeID;
    this->factoryResetRequired = defaultFactoryReset;
}

void ConfigurationMenu::showMenu() {
  int c;

  // Allow the user to enter the configuration menu if Enter is pressed within 5 seconds.
  // Serial.printf("\n%6ld Press Enter within 5 seconds to enter the configuration menu", millis());
  Serial.printf("\nPress Enter within 5 seconds to enter the configuration menu ... ");

  c = waitForEnterOrTimeout(5000);

  // Check if Enter has been pressed.
  if (c != 13) {
    return;
  }

  c = Serial.read(); // ensure CR, LF is read

  for (;;) { // The only way out of this infinite loop is to do a restart.
    // Enter the configuration menu.
    Serial.printf("\nSelect one of the following options;-");
    Serial.printf("\n 1. Restart");
    Serial.printf("\n 2. Configure Node ID");
    Serial.printf("\n 3. Configure WiFi");
    Serial.printf("\n 4. Reset Factory Defaults");
    Serial.printf("\n>");

    // Wait until a key is pressed.
    c = waitForKeyPressed();

    // Check to see which key was pressed.
    switch (c) {
      case 13: // CR received.
        Serial.read(); // read the LF.
        break;

        case 49: // '1' pressed.
        doRestart();
        break;
      
      case 50: // '2' pressed.
        doConfigureNodeID();
        break;
      
      case 51: // '3' pressed.
        doConfigureWiFi();
        break;
      
      case 52: // '4' pressed.
        doResetFactoryDefaults();
        break;
      
      default:
        Serial.printf("\nInvalid input: %d", c);
        break;
    }

    Serial.printf("\n>");
  }
}

NodeID ConfigurationMenu::getNodeID() {
  // Check if a preferences namespace called 'NodeID' exists.
  if (! preferences.begin("NodeID", true)) {
    Serial.printf("\nPreferences namespace NodeID does not exist");
    return this->nodeID; // Return the default Node ID.
  }

  Serial.printf("\nPreferences namespace NodeID does exist");

  // Return the Node ID from preferences.
  uint8_t ID0 = preferences.getUChar("ID0");
  uint8_t ID1 = preferences.getUChar("ID1");
  uint8_t ID2 = preferences.getUChar("ID2");
  uint8_t ID3 = preferences.getUChar("ID3");
  uint8_t ID4 = preferences.getUChar("ID4");
  uint8_t ID5 = preferences.getUChar("ID5");

  preferences.end();

  return NodeID(ID0, ID1, ID2, ID3, ID4, ID5);
}

uint8_t ConfigurationMenu::getFactoryReset() {

  // Check if the preferences namespace 'Reset' exists.
  if (! preferences.begin("Reset", true)) {
    Serial.printf("\nPreferences namespace Reset does not exist");
    return this->factoryResetRequired;
  }

  Serial.printf("\nPreferences namespace Reset does exist");

  uint8_t reset_factory_defaults = preferences.getUChar("Reset"); // is it OK to have the key the same as the namespace ???

  preferences.end();

  // If factory defaults were just reset, then prevent them from being reset again unless set by the user in the configuration menu.
  if (reset_factory_defaults == 1) {
    preferences.begin("Reset");
    preferences.putUChar("Reset", 0);
    preferences.end();
  }

  // Return the factory reset value from preferences.
  return reset_factory_defaults;
}

int ConfigurationMenu::waitForEnterOrTimeout(long timeout) {
  int c = 0;
  long timeoutTime = millis() + timeout;

  do {
    c = Serial.read();
  } while ((c != 13) && (millis() < timeoutTime));

  return c;
}

int ConfigurationMenu::waitForKeyPressed() {
  int c = -1;

  do {
    c = Serial.read(); // Returns -1 if no key pressed.
  } while (c == -1);

  // Echo the key pressed.
  Serial.printf("%c", static_cast<char>(c));

  return c;
}

String ConfigurationMenu::getStringUntilEnter() {
  int c = -1;
  String input = "";

  for (;;) {
    c = waitForKeyPressed();
    if (c == 13) {
      Serial.read(); // Read the LF (c==10).
      break;
    }
    input += (char)c;
  }

  return input;
}

void ConfigurationMenu::doRestart() {
  // Serial.printf("\n Restarting in 5 seconds");
  // delay(5000);
  Serial.printf("\n Restarting ... ");

  // Do the restart here.
  ESP.restart();
}

void ConfigurationMenu::doConfigureNodeID() {
  String input = "";

  Serial.printf("\n Configuring Node ID");
  Serial/printf("\n Changing the node ID will force a factory reset");
  Serial.printf("\nEnter a value for the 6th Node ID value (0 to 255): ");

  input = getStringUntilEnter();
  
  // Check for no input.
  if (input.length() == 0) {
    return;
  }

  // Check for a valid input (i.e. 0 - 255).
  int ID5 = input.toInt();
  if ((ID5 < 0) || (ID5 > 255)) {
    return;
  }

  preferences.begin("NodeID");
  preferences.putUChar("ID0", 5);
  preferences.putUChar("ID1", 1);
  preferences.putUChar("ID2", 1);
  preferences.putUChar("ID3", 1);
  preferences.putUChar("ID4", 0x91);
  // preferences.putUChar("ID5", 0x09); // Third servo node.
  preferences.putUChar("ID5", ID5);
  preferences.end();

  Serial.printf("\n Configured Node ID");

  preferences.begin("Reset");
  preferences.putUChar("Reset", 1);
  preferences.end();

  Serial.printf("\n Set factory reset");

  Serial.printf("\n ===> Note: two restarts will be needed before the Node ID is changed.");
}

void ConfigurationMenu::doConfigureWiFi() {
  Serial.printf("\n Configuring WiFi");

}

void ConfigurationMenu::doResetFactoryDefaults() {
  String input = "";

  Serial.printf("\n Configuring Reset");
  Serial.printf("\nEnter a value for the Reset value (0 or 1): ");

  input = getStringUntilEnter();
  
  // Check for no input.
  if (input.length() == 0) {
    return;
  }

  // Check for a valid input (i.e. 0 - 255).
  int value = input.toInt();
  if ((value < 0) || (value > 1)) {
    return;
  }

  preferences.begin("Reset");
  preferences.putUChar("Reset", value);
  preferences.end();

  Serial.printf("\n Configured Reset");
}
