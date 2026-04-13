#if DISPLAY_TYPE_SH110X
#ifndef VIEW_MANAGER_SH1107_H
#define VIEW_MANAGER_SH1107_H

// ===================== SCREEN configuration =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#include <stdint.h>
#include "../core/state_manager.h"
#include "../sensors/sensor_manager.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include "../../include/utils/icons.h"
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/bluetooth_conn.h"
#include "../../include/connectivity/wifi_conn.h"
#endif
#if LED_MANAGER_ENABLED
#include "../../include/lights/LEDManager.h"
extern LEDManager ledManager;
#endif

class Adafruit_SH1107;
class ViewManager {
public:
    void begin(Adafruit_SH1107& disp);
    void draw();
    void setView(ViewID view) { _currentView = view; }
    ViewID getCurrentView() const { return _currentView; }
private:
    void drawSplash();
    void drawStandby();
    void drawInfo();
    void drawTempConfig();
    void drawHumConfig();
    void drawModeConfig();
    void drawDryTimeConfig();
    void drawDryTimeStartConfig();
    void TempSensorError();
    void drawTemperature();
    void drawHumidity();
    void drawFanStatus();
    void drawHeaterStatus();
    void drawLEDStatus();
    void drawPowerOutageMemory();
	void drawScreenSaverSetting();
	void StartSubScreens(const char* title);
    void ScreenSaverOn();
    void SaveEnterText();
    void EditPartOfSubScreen(const char* label);
    void TimeConfiguration(uint32_t TimeVariable);
#if BLUETOOTH_WIFI_ENABLED
    void drawBTScanning();
    void drawBTConnected();
    void drawWifiScanning();
    void drawWifiSelect();
    void drawWifiConnected();
    void wifiStatusSymbol();
#endif


    Adafruit_SH1107* _display = nullptr;
    ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_H 

#endif // DISPLAY_TYPE_SH110X