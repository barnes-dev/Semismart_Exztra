#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED
#ifndef CONNECTIVITY_BLUETOOTH_H
#define CONNECTIVITY_BLUETOOTH_H

#include <Arduino.h>
#include <string.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEClient.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "wifi_conn.h"

// Specialized modules
#if DISPLAY_TYPE_SSD1306
#include "../../include/display/view_manager_sh1306.h"

#elif DISPLAY_TYPE_SH110X
#include "../../include/display/view_manager_sh1107.h"

#elif DISPLAY_TYPE_SSD1351
#include "../../include/display/view_manager_ssd1351.h"

#elif DISPLAY_TYPE_ST7796S
#include "../../include/display/view_manager_st7796s.h"
#endif


/*
	Video: https://www.youtube.com/watch?v=oCMOYS71NIU
	Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
	Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BTExztra {

public:
void connectReceive();
void startBTSetup();
void resetConnectTimeout();
void resetBluetooth();
bool isDeviceConnected();
bool isConnectTimeout();
std::string getDeviceName();
std::string getReceivedValue();


private:
};

extern BTExztra btExztra;

#endif // CONNECTIVITY_BLUETOOTH_H

#endif // BLUETOOTH_ENABLED
