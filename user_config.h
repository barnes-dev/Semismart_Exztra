#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/* PSEUDOCODE / PLAN (preprocessor comment):
   - After the available DISPLAY_TYPE_* macros are defined (each 0 or 1),
     compute a macro DISPLAY_TYPE_COUNT that sums them.
   - Use preprocessor checks to enforce that exactly one display type is enabled:
     - If DISPLAY_TYPE_COUNT == 0 -> produce #error: "No display type selected..."
     - If DISPLAY_TYPE_COUNT > 1 -> produce #error: "Multiple display types selected..."
   - Keep all original configuration and notes intact.
   - Place the checks immediately after the display type macros so a clear compile-time
     error points to the configuration issue before other code is processed.
*/

 // ===================== Display configuration =====================
 // Supported displays: Adafruit_SSD1306, Adafruit_SH110X, Adafruit ST7796S, Adafruit SSD1351
 // (Set 1 for the one you are using, and 0 for the others to save memory)
 #define DISPLAY_TYPE_SSD1306  0
 #define DISPLAY_TYPE_SH110X   1
 #define DISPLAY_TYPE_SSD1351  0
 #define DISPLAY_TYPE_ST7796S  0

// Compile-time check: ensure exactly one display type is set to 1.
#define DISPLAY_TYPE_COUNT (DISPLAY_TYPE_SSD1306 + DISPLAY_TYPE_SH110X + DISPLAY_TYPE_SSD1351 + DISPLAY_TYPE_ST7796S)

#if DISPLAY_TYPE_COUNT == 0
  #error "No display type selected. Set exactly one DISPLAY_TYPE_* to 1 in user_config.h"
#elif DISPLAY_TYPE_COUNT > 1
  #error "Multiple display types selected. Set exactly one DISPLAY_TYPE_* to 1 in user_config.h"
#endif

// Note: If using Adafruit_SSD1306, make sure to adjust the display initialization in view_manager.cpp accordingly (SCREEN_WIDTH, SCREEN_HEIGHT, and constructor parameters).
// Note: If using Adafruit_SH110X, make sure to adjust the display initialization in view_manager.cpp accordingly (SCREEN_WIDTH, SCREEN_HEIGHT, and constructor parameters).
// Note: The Adafruit_ST7796S library is designed for 16-bit color displays, so if you enable it, 
//		make sure to adjust the display initialization in view_manager.cpp to use the 
//		correct constructor and parameters for that library. Also, keep in mind that using 
//		a full-color TFT display will consume more memory and may require optimizations in 
//		the view_manager to ensure smooth performance on the Arduino Nano.
// Note: The Adafruit_SSD1351 library is designed for 16-bit color displays, so if you enable it, 
//		make sure to adjust the display initialization in view_manager.cpp to use the 
//		correct constructor and parameters for that library. Also, keep in mind that using 
//		a full-color TFT display will consume more memory and may require optimizations in 
//		the view_manager to ensure smooth performance on the Arduino Nano.
// The FreeSansBold18pt7b font is included because it is used in the view_manager for certain screens, but if you switch to a display that doesn't support it or if you want to save memory, you can comment it out and use a different font or the default one provided by the display library.
// #define DISPLAY_USE_FREESANSBOLD18PT7B 1
// 
// ===================== USER CONFIGURABLE SETTINGS =====================
// We can enable additional features like LED animations. Disable if you don't intend 
// to use LED strip to save memory for other features.
#define LED_MANAGER_ENABLED 0	// Enable LED animations (requires WS2812B LEDs)



// ======================================================================

// ===================== GPIO Interface (Prusa) Integration =====================
// Disable for printer without GPIO interface eg. MK3 series
#define GPIO_INTERFACE_ENABLED 1
// ==============================================================================

// ===================== Unit configuration =====================
// Temperature unit: Celsius only

// ===================== Fan PWM speeds (Configurable) =====================
// Recommended values for minimum noise and maximum efficiency (0 to 255 max)
#define FAN_SPEED_STANDBY_CYCLING_PWM 255  // 40% - Standby cycling
#define FAN_SPEED_NORMAL_CYCLING_PWM 255   // 60% - Normal operation cycling (BY TIME/BY HUM)
#define FAN_MIN_PWM_WHEN_HEATING 170        // 35% - Minimum fan speed when heater is active

// ===================== Fan control timing =====================
#define FAN_COOLDOWN_TIME 45000     // Cooldown time after heater off
#define FAN_CYCLE_ON_TIME 15000      // Standby cycle on time
#define FAN_CYCLE_OFF_TIME 300000    // Standby cycle off time
#define FAN_TRANSITION_TIME 2000     // Smooth transition time (2 seconds)

#endif // USER_CONFIG_H
