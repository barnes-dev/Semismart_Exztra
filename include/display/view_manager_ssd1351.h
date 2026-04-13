#if DISPLAY_TYPE_SSD1351
#ifndef VIEW_MANAGER_SSD1351_H
#define VIEW_MANAGER_SSD1351_H

// ===================== SCREEN configuration =====================
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   128
#define CS_PIN          A0
#define RST_PIN         A1
#define DC_PIN          A2
#define SCLK_PIN        2
#define MOSI_PIN        4

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	PINK            0xFB54
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF


#include <stdint.h>
#include "../core/state_manager.h"
#include "../sensors/sensor_manager.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Wire.h>
#include <Adafruit_SSD1351.h>
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

class Adafruit_SSD1351;
class ViewManager {
public:
    void begin(Adafruit_SSD1351& disp);
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
    void drawScreenSaverSetting();
    void drawPowerOutageMemory();
    void ScreenSaverOn();
    void StartSubScreens(const char* title);
    void SaveEnterText();
#if BLUETOOTH_WIFI_ENABLED
    void drawBTScanning();
    void drawBTConnected();
    void drawWifiScanning();
    void drawWifiSelect();
    void drawWifiConnected();
    void wifiStatusSymbol();
#endif

    Adafruit_SSD1351* _display = nullptr;
    ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_SSD1351_H 

#endif // DISPLAY_TYPE_SSD1351