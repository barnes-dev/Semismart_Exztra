#ifndef STRINGS_H
#define STRINGS_H

#include <Arduino.h>
#include <avr/pgmspace.h>

#if DISPLAY_TYPE_SSD1306
#include <Adafruit_SSD1306.h>		 
class Adafruit_SSD1306;
extern Adafruit_SSD1306 display;

#elif DISPLAY_TYPE_SH110X
#include <Adafruit_SH110X.h>
class Adafruit_SH1107;
extern Adafruit_SH1107 display;

#elif DISPLAY_TYPE_SSD1351
#include <Adafruit_SSD1351.h>
class Adafruit_SSD1351;
extern Adafruit_SSD1351 display;

#elif DISPLAY_TYPE_ST7796S
#include <Adafruit_ST7796S.h>
class Adafruit_ST7796S;
extern Adafruit_ST7796S display;
#endif
extern bool displayAvailable;

// Navigation strings
static const char PROGMEM str_cooling[] = "COOLING";
static const char PROGMEM str_cycling[] = "CYCLING";
static const char PROGMEM str_system_off[] = "SYS OFF";
static const char PROGMEM str_heat_off[] = "HEAT OFF";
static const char PROGMEM str_fan_off[] = "FAN OFF";
static const char PROGMEM str_standby[] = "Standby";
static const char PROGMEM str_screen_saver_never[] = "OFF";

// Operation mode strings
static const char PROGMEM str_printing[] = "PRINTING";
static const char PROGMEM str_drying[] = "DRYING";
static const char PROGMEM str_warning[] = "WARN";
static const char PROGMEM str_balance[] = "BALANCE";

// Configuration strings
static const char PROGMEM str_set_temp[] = "Set Temp";
static const char PROGMEM str_change_save[] = "+/-:Chg O:Save";
static const char PROGMEM str_set_humidity[] = "Set Humidity";
static const char PROGMEM str_operation[] = "Operation";
static const char PROGMEM str_mode[] = "Mode";
static const char PROGMEM str_operation_mode[] = "Op Mode";
static const char PROGMEM str_power_loss_memory_one[] = "Mode After";
static const char PROGMEM str_power_loss_memory_two[] = "Power Return";
static const char PROGMEM str_power_off[] = "Power Off";
static const char PROGMEM str_last_mode[] = "Last Mode";
static const char PROGMEM str_by_hum[] = "By Humid";
static const char PROGMEM str_dry[] = "Dry Timer";
static const char PROGMEM str_screen_saver[] = "Screen Saver";

// Control mode strings
static const char PROGMEM str_mode_control[] = "Mode:";
static const char PROGMEM str_auto_temp[] = "AUTO TEMP";
static const char PROGMEM str_auto_hum[] = "AUTO HUM";
static const char PROGMEM str_user_temp[] = "USER TEMP";
static const char PROGMEM str_user_hum[] = "USER HUM";

// Dry mode string
static const char PROGMEM str_dry_mode_time[] = "Set time";

// Units
static const char PROGMEM str_celsius[] = "C";
static const char PROGMEM str_percent[] = "%";
static const char PROGMEM str_seconds[] = " sec";
static const char PROGMEM str_minutes[] = " min";

// Display labels
static const char PROGMEM str_temp_label[] = "Temperature ";
static const char PROGMEM str_humidity_label[] = "Humidity ";
static const char PROGMEM str_led_pattern_label[] = "LED Pattern";
static const char PROGMEM str_brightness_level[] = "Brightness";
static const char PROGMEM str_eta_label[] = "ETA";
static const char PROGMEM str_dry_time_start[] = "Drying starts in";
static const char PROGMEM str_heat_format[] = "Heat %d%%";
static const char PROGMEM str_heater_format[] = "Heat";
static const char PROGMEM str_fan_format[] = "Fan speed";
static const char PROGMEM str_space[] = " ";
static const char PROGMEM str_greater[] = " -> ";
static const char PROGMEM str_percent_sign[] = "%";

// Helper functions for PROGMEM strings		  
#if DISPLAY_TYPE_SSD1306
inline void printProgmemString(Adafruit_SSD1306* disp, const char* str) {
		if (!disp) return;
  char buffer[20];
  strcpy_P(buffer, str);
  disp->print(buffer);
}

#elif DISPLAY_TYPE_SH110X
inline void printProgmemString(Adafruit_SH1107* disp, const char* str) {
  if (!disp) return;
  char buffer[20];
  strcpy_P(buffer, str);
  disp->print(buffer);
}

#elif DISPLAY_TYPE_SSD1351
inline void printProgmemString(Adafruit_SSD1351* disp, const char* str) {
	if (!disp) return;
	char buffer[20];
	strcpy_P(buffer, str);
	disp->print(buffer);
}

#elif DISPLAY_TYPE_ST7796S
inline void printProgmemString(Adafruit_ST7796S* disp, const char* str) {
  if (!disp) return;
  char buffer[20];
  strcpy_P(buffer, str);
  disp->print(buffer);
}
#endif


inline const char* getProgmemString(const char* progmemString) {
  static char buffer[20];
  strcpy_P(buffer, progmemString);
  return buffer;
}

// Logo strings
static const char PROGMEM str_voxel3d[] = "VOXEL3D";
#if DISPLAY_TYPE_SH110X || DISPLAY_TYPE_SSD1351
static const char PROGMEM str_code_by[] = "Code by";
static const char PROGMEM str_3dmonkey[] = "3DMonkey";
#else
static const char PROGMEM str_code_by[] = "Code by 3DMonkey";
#endif

#endif // STRINGS_H 