#if DISPLAY_TYPE_SSD1306
#ifndef VIEW_MANAGER_SH1306_H
#define VIEW_MANAGER_SH1306_H

// ===================== SCREEN configuration =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C

#include <stdint.h>
#include "../core/state_manager.h"
#include "../sensors/sensor_manager.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/bluetooth_conn.h"
#include "../../include/connectivity/wifi_conn.h"
#endif

class Adafruit_SSD1306;
class ViewManager {
    
public:
    void begin(Adafruit_SSD1306& disp);
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
    void drawScreenSaverSetting();
    void TempSensorError();
    void drawPowerOutageMemory();
    void SaveEnterText();
	void ScreenSaverOn();
	void StartSubScreens(const char* title, uint8_t xpos);
	void TimeConfiguration(uint32_t TimeVariable);
#if BLUETOOTH_WIFI_ENABLED
    void drawBTScanning();
    void drawBTConnected();
    void drawWifiScanning();
    void drawWifiSelect();
    void drawWifiConnected();
    void wifiStatusSymbol();
#endif

    Adafruit_SSD1306* _display = nullptr;            
    ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_SH1306_H 

#endif  // DISPLAY_TYPE_SSD1306