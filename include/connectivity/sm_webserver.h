#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED
#ifndef SM_WEBSERVER_H
#define SM_WEBSERVER_H

#include <Arduino.h>
#include <pgmspace.h>
#include <string.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "../../include/utils/strings.h"
#include "../../include/core/system_config.h"
#include "../../include/core/state_manager.h"
#include "../../include/sensors/sensor_manager.h"
#include "../../include/utils/storage.h"
#include "../../include/control/control.h"
#include "../../include/connectivity/rootpage.h"
#include "../../include/sensors/print_status_sensor.h"
#if LED_MANAGER_ENABLED
#include "../../include/lights/LEDManager.h"
// reference global instance defined in main sketch
extern LEDManager ledManager;
#endif

class SemiSmartWebServer {
public:
  void smwebsetup(void);
  void smwebloop(void);
  void handleRoot();
  void handleNotFound();
  void drawGraphen();
  void handlePower();
  void handleSensors();  // added: return temperature/humidity JSON
  void handleTargetTemp();
  void handleTargetHumidity();
  void handleSetMode();
  void handleSetControlMode();
  void handleSetDryDuration();
  void handleSetIdleStart();
  void handleSetScreenSaver();
  void handleSetPowerLossMemory();
  #if LED_MANAGER_ENABLED
  void handleSetLedPattern();
  void handleSetLedBrightness();
  #endif
  void handleUpdate();           // final POST completion handler
  void handleUpdateUpload();     // upload processing handler
// Added LED control handlers (when LED_MANAGER_ENABLED)
};

// Small no-op Print implementation to avoid using Serial while preserving Update.printError API.
class NullPrint : public Print {
public:
	size_t write(uint8_t ch) override { (void)ch; return 1; }
	size_t write(const uint8_t* buffer, size_t size) override { (void)buffer; return size; }
};


extern "C" void turnSystemOff();
extern "C" void turnSystemStandby();
extern "C" void turnSystemOn();


extern SemiSmartWebServer sswServer;

#endif  // SM_WEBSERVER_H

#endif  // BLUETOOTH_WIFI_ENABLED
