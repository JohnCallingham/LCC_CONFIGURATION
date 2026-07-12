# LCC_CONFIGURATION
A software component which can be used as part of an OpenLCB/LCC node. It has been developed using PlatformIO for an Arduino Nano ESP32. The LCC node is designed to connect over WiFi to the LCC hub provided by JMRI.

## Purpose

It has been developed to allow a user to configure multiple nodes prior to and after deployment without needing physical access to the node. The user can remotely configure the following options;-
- Allows a node to work with multiple WiFi networks (not concurrently) for configuration
- Set the required Node ID
- Set the WiFi network to which the node connects to access JMRI
- Update a node's firmware


## Files

These files are involved;-
- **ConfigurationOTA**. A class which allows a configuration file in json format to be hosted on a web server, external to the nodes, which provides a way for a user to alter various configuration properties of multiple nodes without needing physical access to the nodes.
- **ConfigurationPreferences**. A namespace which provides support methods to allow various configuration data to be stored and retrieved using the Arduino Preferences system.
- **credentials.h**. This file contains credential details (SSID and password) regarding available WiFi networks. This allows for development to take place in various locations and will use the available WiFi network in the current location. It also contains the URL of a configuration file, if one is accessible from that WiFi network.
- **configuration.json**. This file contains details of all LCC nodes.

## Operation

Every time a node starts the process is as follows;-
1. The json formatted contents of the hard coded credentials.h file is read and deserialised. This file contains the credentials of one or more WiFi networks to which the node may be able to connect.
2. A list is made of those WiFi networks which have a configuration_url stored.
3. Each of these WiFi networks is tried in turn until one is found which is accessible.
4. A connection is made to the first accessible WiFi network in the above list.
5. The json configuration file indicated by configuration_url is downloaded and deserialised.
6. Of the many records in the configuration file there will be only one which matches this node's MAC address and this record is used to configure the node.
7. If the Node ID stored in this record in the configuration file is different to the Node ID currently used by the node, the new Node ID is stored in Preferences to be used later when the node initialises.
8. If the value of ```[Update][Version]``` in the json configuration file is different to that which is currently installed, then the file located at ```<[Update][Path]>/V<[Update][Version]>/<[Update][Filename]>``` is downloaded and installed. The ESP32 is then restarted to run the new firmware.
9. If no new firmware has been downloaded, then processing continues.
10. The WiFi connection which has downloaded the configuration file is disconnected.
11. The configuration record for this node will contain the name of the WiFi network which the node will use to connect to JMRI.
12. The credentials for this WiFi network are looked up from the credentials file and used to connect to JMRI.
13. The node will connect to JMRI and continue its initialisation using the Node ID which has been stored in Preferences.
14. If the user wishes that all the node's events IDs are recalculated to reflect the new Node ID, then the user should select the 'Clear Everything back to Factory Defaults' option in JMRI's CLI editor before restarting the node.

## Example json from credentials.h
This file is hard coded into all nodes' software.
```json
{
  "Credentials":
  [
    {
      "name":"RPi",
      "ssid":"RPi-JMRI",
      "password":"rpI-jmri"
    },
    {
      "name":"London",
      "ssid":"BT-XXXXPC",
      "password":"xyzzy",
      "configuration_url":"http://diskstation2/LCC_Configuration/Configurations.json"
    },
    {
      "name":"Dorset",
      "ssid":"BT-XXXXQF",
      "password":"xyzzy",
      "configuration_url":"http://diskstation3/LCC_Configuration/Configurations.json"
    }
  ]
}
```

## Example Configurations.json file
This file is placed on all web servers that are specified in credentials.h

```json
{
  "Configurations":
  [
    {
      "MAC_Address": "3C:84:27:C4:A7:B8",
      "Board": "ESP32_6TOTI_WiFi",
      "NodeID": "05.01.01.01.91.0A",
      "Update": {
        "Version": "1.0.6",
        "Path": "https://raw.githubusercontent.com/JohnCallingham/ESP32_6TOTI_WiFi/main/firmware",
        "Filename": "firmware.bin"
      },
      "JMRI_name": "Dorset"
    },
    {
      "MAC_Address": "74:4D:BD:A0:FC:5C",
      "Board": "ESP32_2Servo_2Frog_2TOTI_WiFi",
      "NodeID": "05.01.01.01.91.09",
      "Update": {
        "Version": "1.0.6",
        "Path": "https://raw.githubusercontent.com/JohnCallingham/ESP32_2Servo_2Frog_2TOTI_WiFi/main/firmware",
        "Filename": "firmware.bin"
      },
      "JMRI_name": "Dorset"
    }
  ]
}
```






