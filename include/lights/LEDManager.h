#include "../../user_config.h"
#if LED_MANAGER_ENABLED
#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>

class LEDManager {
public:
  LEDManager(uint8_t pin, uint16_t numLeds);

  void begin();
  void update();  // Call every loop()

  void nextPattern();
  void nextBrightness();
  void setPattern(uint8_t ledIndex);
  void setBrightnessLevel(uint8_t brightnessIndex);


  const char* getCurrentPatternName() const;
  uint8_t getPatternIndex() const;
  uint8_t getBrightnessLevel() const;

private:
  void solidWhite();
  void pulsingWhite();
  void rainbow();
  void rainbowChase();  
  void fireFlicker();
  void plasma();
  void sparkle();
  void sparkleColorful();
  void auroraFade();
  void colorDrift();
  void spookyFlicker();
  void ghostPulse();
  void lightningStorm();
  void creepyCrawl();
  void zombieMarch();
  void eyesInTheDark();

  Adafruit_NeoPixel strip;

  uint8_t patternIndex;
  uint8_t brightnessLevelIndex;
  
  uint16_t baseHue;

  unsigned long pulseStartTime;
  unsigned long lastUpdate;

  static const uint8_t NUM_PATTERNS = 16;
  static const uint8_t NUM_BRIGHTNESS_LEVELS = 5;
  const uint8_t brightnessLevels[NUM_BRIGHTNESS_LEVELS] = { 0, 50, 100, 150, 200 };

  // Human-readable pattern names (for OLED or serial)
  static constexpr const char* PATTERN_NAMES[NUM_PATTERNS] = {
    "Solid White",
    "Pulsing White",
    "Rainbow",
    "Rainbow Chase",
    "Fire Flicker",
    "Plasma",
    "Sparkle",
    "Sparkle Colorful",
    "Aurora Fade",
    "Color Drift",
    "Spooky Flicker",
    "Ghost Pulse",
    "Lightning Storm",
    "Creepy Crawl",
    "Zombie March",
    "Eyes in the Dark"
  };

  // EEPROM Debounce settings
  bool settingsChanged = false;
  unsigned long settingsChangeTime = 0;
  const unsigned long EEPROM_SAVE_DELAY = 10000;  // 10 seconds
};

#endif // LED_MANAGER_H
#endif // LED_MANAGER_ENABLED
