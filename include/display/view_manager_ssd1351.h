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
    void ScreenSaverOn();
    void drawPowerOutageMemory();
    void StartSubScreens(const char* title);
    void SaveEnterText();

    Adafruit_SSD1351* _display = nullptr;
    ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_SSD1351_H 

#endif // DISPLAY_TYPE_SSD1351