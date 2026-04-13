#if DISPLAY_TYPE_ST7796S
#ifndef VIEW_MANAGER_ST7796S_H
#define VIEW_MANAGER_ST7796S_H

// ===================== SCREEN configuration =====================
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   480
#define CS_PIN          A0
#define RST_PIN         A1
#define DC_PIN          A2
#define SCLK_PIN        2
#define MOSI_PIN        4

// Some ready-made 16-bit ('565') color settings:
#define ST7796_BLACK    0xFFFF
#define ST7796_WHITE    0x0000
#define ST7796_YELLOW   0xF800
#define ST7796_PINK     0x0700
#define ST7796_GREEN    0xF0FF
#define ST7796_CYAN     0x001F
#define ST7796_BLUE     0x07FF
#define ST7796_MAGENTA  0x1F8F
#define ST7796_RED      0xFF00
#define ST7796_ORANGE   0xFD00

#include <stdint.h>
#include "../core/state_manager.h"
#include "../sensors/sensor_manager.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Adafruit_ST7796S.h>
//#include <Fonts/FreeSansBold18pt7b.h>  // A custom font
#include "../../include/utils/icons.h"
#include <cstring>
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/bluetooth_conn.h"
#include "../../include/connectivity/wifi_conn.h"
#endif
#if LED_MANAGER_ENABLED
#include "../../include/lights/LEDManager.h"
extern LEDManager ledManager;
#endif

class Adafruit_ST7796S;
class ViewManager {
public:
    void begin(Adafruit_ST7796S& disp);
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
    void StartSubScreens(const char* title, uint8_t textSize);
    void SecondTitleRow(const char* secondTitle);
    void SaveEnterText();
#if BLUETOOTH_WIFI_ENABLED
    void drawBTScanning();
	void drawBTConnected();
	void drawWifiScanning();
	void drawWifiSelect();
	void drawWifiConnected();
    void wifiStatusSymbol();
#endif

   Adafruit_ST7796S* _display = nullptr;
   ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_ST7796S_H 

#endif // DISPLAY_TYPE_ST7796S