#include "../../include/lights/LEDManager.h"
#include "../../user_config.h"
#if ARDUINO_ESP32
#include <Preferences.h>

Preferences LED_EEPROM;

LEDManager::LEDManager(uint8_t pin, uint16_t numLeds)
	: strip(numLeds, pin, NEO_RGB + NEO_KHZ800),
	patternIndex(2),
	brightnessLevelIndex(4),  // Default to max brightness    
	baseHue(0),
	pulseStartTime(0),
	lastUpdate(0),
	settingsChanged(false),
	settingsChangeTime(0),
	EEPROM_SAVE_DELAY(10000) // 10 seconds
{}

void LEDManager::begin() {
	LED_EEPROM.begin("led_storage", false);  // Use a separate namespace for LED settings
	strip.begin();

	randomSeed(analogRead(A0));  // or any unused analog pin | Critical to keep from freezing/locking up | LED Patterns that need it: FireFlicker

	// Read from EEPROM
	uint8_t storedPattern = LED_EEPROM.getUInt("PATTERN_ADDR");
	uint8_t storedBrightness = LED_EEPROM.getUInt("BRIGHTNESS_ADDR");

	// Validate and apply stored pattern
	if (storedPattern < NUM_PATTERNS) {
		patternIndex = storedPattern;
	}
	else {
		patternIndex = 2; // fallback to rainbow
		LED_EEPROM.putUInt("PATTERN_ADDR", patternIndex);
	}

	// Validate and apply stored brightness
	if (storedBrightness < NUM_BRIGHTNESS_LEVELS) {
		brightnessLevelIndex = storedBrightness;
	}
	else {
		brightnessLevelIndex = NUM_BRIGHTNESS_LEVELS - 1; // fallback to max
		LED_EEPROM.putUInt("BRIGHTNESS_ADDR", brightnessLevelIndex);
	}

	strip.setBrightness(brightnessLevels[brightnessLevelIndex]);
	strip.show();

	pulseStartTime = millis();
	LED_EEPROM.end();  // Close EEPROM after reading/writing
}

void LEDManager::update() {
	unsigned long now = millis();
	if (now - lastUpdate < 20) return;  // Limit update rate
	lastUpdate = now;
	LED_EEPROM.begin("led_storage", false);  // Use a separate namespace for LED settings

	switch (patternIndex) {
	case 0: solidWhite(); break;
	case 1: pulsingWhite(); break;
	case 2: rainbow(); break;
	case 3: rainbowChase(); break;
	case 4: fireFlicker(); break;
	case 5: plasma(); break;
	case 6: sparkle(); break;
	case 7: sparkleColorful(); break;
	case 8: auroraFade(); break;
	case 9: colorDrift(); break;
	case 10: spookyFlicker(); break;
	case 11: ghostPulse(); break;
	case 12: lightningStorm(); break;
	case 13: creepyCrawl(); break;
	case 14: zombieMarch(); break;
	case 15: eyesInTheDark(); break;
	}

	// Save settings to EEPROM if changed and 10s passed
	if (settingsChanged && now - settingsChangeTime >= EEPROM_SAVE_DELAY) {
		LED_EEPROM.putUInt("PATTERN_ADDR", patternIndex);
		LED_EEPROM.putUInt("BRIGHTNESS_ADDR", brightnessLevelIndex);
		settingsChanged = false;
	}
	LED_EEPROM.end();  // Close EEPROM after reading/writing
}

void LEDManager::nextPattern() {
	uint8_t newIndex = (patternIndex + 1) % NUM_PATTERNS;
	if (newIndex != patternIndex) {
		patternIndex = newIndex;
		settingsChanged = true;
		settingsChangeTime = millis();

		// Reset animation state for some patterns
		pulseStartTime = millis();
	}
}

void LEDManager::nextBrightness() {
	uint8_t newIndex = (brightnessLevelIndex + 1) % NUM_BRIGHTNESS_LEVELS;
	if (newIndex != brightnessLevelIndex) {
		brightnessLevelIndex = newIndex;
		strip.setBrightness(brightnessLevels[brightnessLevelIndex]);
		strip.show();
		settingsChanged = true;
		settingsChangeTime = millis();
	}
}

const char* LEDManager::getCurrentPatternName() const {
	if (patternIndex < NUM_PATTERNS) {
		return PATTERN_NAMES[patternIndex];
	}
	return "Unknown";
}

uint8_t LEDManager::getPatternIndex() const {
	return patternIndex;
}

uint8_t LEDManager::getBrightnessLevel() const {
	return brightnessLevelIndex;
}

void LEDManager::solidWhite() {
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(255, 255, 255));
	}
	strip.show();
}

void LEDManager::pulsingWhite() {
	float time = (millis() - pulseStartTime) / 1000.0;
	float speed = 0.2;
	float brightness = (sin(time * TWO_PI * speed) + 1.0) * 0.5 * 255;
	uint8_t b = (uint8_t)brightness;

	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(b, b, b));
	}
	strip.show();
}

void LEDManager::rainbow() {
	baseHue += 256;
	const uint8_t colorDensity = 4;

	for (int i = 0; i < strip.numPixels(); i++) {
		uint16_t pixelHue = baseHue + (i * 65536L * colorDensity / strip.numPixels());
		strip.setPixelColor(i, strip.ColorHSV(pixelHue, 255, 255));
	}
	strip.show();
}

void LEDManager::fireFlicker() {
	unsigned long now = millis();

	// Update every 50ms
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Flicker brightness between 150 and 255
		uint8_t flicker = random(150, 256);
		strip.setPixelColor(i, strip.Color(flicker, flicker / 3, 0)); // warm orange-yellow
	}
	strip.show();
}

void LEDManager::sparkle() {
	unsigned long now = millis();

	// Limit update rate (e.g. every 50ms)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	// Fade all pixels slightly
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		// Dim each color slightly
		r = r * 0.8;
		g = g * 0.8;
		b = b * 0.8;

		strip.setPixelColor(i, r, g, b);
	}

	// Light up a few random sparkles
	const uint8_t sparkleCount = strip.numPixels() / 20;  // ~5% of LEDs
	for (int i = 0; i < sparkleCount; i++) {
		int pixel = random(0, strip.numPixels());
		uint8_t brightness = random(150, 256);
		strip.setPixelColor(pixel, strip.Color(brightness, brightness, brightness)); // white sparkle
	}

	strip.show();
}

void LEDManager::sparkleColorful() {
	unsigned long now = millis();

	// Limit update rate (e.g. every 50ms)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	// Fade all pixels slightly
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		r = r * 0.8;
		g = g * 0.8;
		b = b * 0.8;

		strip.setPixelColor(i, r, g, b);
	}

	// Add random colorful sparkles
	const uint8_t sparkleCount = strip.numPixels() / 20;  // ~5% sparkle
	for (int i = 0; i < sparkleCount; i++) {
		int pixel = random(0, strip.numPixels());
		uint8_t hue = random(0, 256);
		uint32_t color = strip.ColorHSV(hue * 256, 255, 255);  // full sat/val
		strip.setPixelColor(pixel, color);
	}

	strip.show();
}

void LEDManager::rainbowChase() {
	unsigned long now = millis();

	// Update every 50ms
	static unsigned long lastUpdate = 0;
	static uint16_t chaseOffset = 0;  // Animation offset
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	chaseOffset++;  // Moves pattern forward

	const uint8_t stripeLength = 10;  // How many LEDs per cycle
	const uint8_t colorDensity = 4;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Create a moving pattern using offset
		int pos = (i + chaseOffset) % (stripeLength * 2);

		if (pos < stripeLength) {
			// Only light part of the pattern
			uint16_t hue = (i * 65536L * colorDensity / strip.numPixels()) + (chaseOffset * 256);
			strip.setPixelColor(i, strip.ColorHSV(hue, 255, 255));
		}
		else {
			// Leave other half dark
			strip.setPixelColor(i, 0);
		}
	}

	strip.show();
}

void LEDManager::plasma() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 30) return;
	lastUpdate = now;

	float speed = 0.0015;  // how fast the waves move
	float t = millis() * speed;

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();  // 0.0 to 1.0
		float wave = sin(2 * PI * (pos * 3.0 - t));  // 3 waves across strip

		// Brightness between 0 and 255
		uint8_t brightness = (wave + 1.0) * 0.5 * 255;

		// Shift hue over time
		uint16_t hue = ((uint16_t)(t * 2000) + i * 30) % 65536;

		uint32_t color = strip.ColorHSV(hue, 255, brightness);
		strip.setPixelColor(i, color);
	}

	strip.show();
}

void LEDManager::auroraFade() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;  // update every 50ms
	lastUpdate = now;

	float time = now / 10000.0;  // slow drift

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();

		// Create smooth brightness pulsing across the strip
		float wave = sin(2 * PI * (pos * 2.0 + time));  // 2 waves across strip

		uint8_t brightness = (wave + 1.0) * 0.5 * 255;

		// Use a drifting hue within a cool range (green/blue/purple)
		uint16_t baseHue = 16000;  // green-blue start
		uint16_t hueRange = 15000; // spread across cool tones
		uint16_t hue = baseHue + (uint16_t)(sin(time + pos * 2.0) * hueRange);

		strip.setPixelColor(i, strip.ColorHSV(hue, 180, brightness));  // softer saturation
	}

	strip.show();
}

void LEDManager::colorDrift() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;  // update every 50ms
	lastUpdate = now;

	float time = now / 10000.0;  // slower = smoother

	// Slowly change hue over time (wraps automatically at 65535)
	uint16_t hue = (uint16_t)(sin(time * TWO_PI) * 32767 + 32768);

	// Apply this color to all pixels
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.ColorHSV(hue, 200, 255));
	}

	strip.show();
}

void LEDManager::spookyFlicker() {
	static unsigned long lastUpdate = 0;
	unsigned long now = millis();
	if (now - lastUpdate < 50) return; // Update ~20 FPS
	lastUpdate = now;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Random warm flicker color
		uint8_t flicker = random(120, 255);
		uint8_t red = flicker;
		uint8_t green = flicker * random(30, 60) / 100;  // dimmer green
		uint8_t blue = random(0, 20);  // very subtle blue or none

		// Occasionally (very rarely), flash a ghostly blue-white (eerie effect)
		if (random(0, 1000) < 2) {  // ~0.2% chance per pixel per frame
			red = 200;
			green = 200;
			blue = 255;
		}

		strip.setPixelColor(i, red, green, blue);
	}

	strip.show();
}

void LEDManager::ghostPulse() {
	unsigned long now = millis();

	// Frame rate limiting (~40 FPS)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 25) return;
	lastUpdate = now;

	float time = now / 1000.0;  // seconds
	float speed = 0.15;         // slower pulse
	float pulse = (sin(time * TWO_PI * speed) + 1.0) * 0.5;  // 0.0 to 1.0

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();

		// Slight hue shift across strip and over time
		uint16_t baseHue = 18000;  // cold blue
		uint16_t hueRange = 8000;  // into violet
		uint16_t hue = baseHue + (uint16_t)(sin(time + pos * 3.0) * hueRange);

		// Random ghost flash (very rare)
		bool ghostFlash = random(0, 2000) < 2;  // ~0.1% chance per pixel/frame

		uint8_t brightness = pulse * 180 + random(0, 20);  // flickery but soft

		if (ghostFlash) {
			// Spectral white-blue flash
			strip.setPixelColor(i, strip.Color(200, 200, 255));
		}
		else {
			// Soft blue/violet ghost glow
			uint32_t color = strip.ColorHSV(hue, 100, brightness);  // low sat
			strip.setPixelColor(i, color);
		}
	}

	strip.show();
}

void LEDManager::lightningStorm() {
	static unsigned long lastFlash = 0;
	static unsigned long nextFlashDelay = 0;
	static uint8_t flashCount = 0;
	static bool flashing = false;

	unsigned long now = millis();

	if (!flashing && now - lastFlash >= nextFlashDelay) {
		// Start a lightning sequence
		flashing = true;
		flashCount = random(2, 5);  // More flashes per storm
		lastFlash = now;
	}

	if (flashing) {
		static bool flashOn = false;
		static unsigned long flashTime = 0;

		// Flash durations — much faster and punchier
		if (now - flashTime > (flashOn ? random(30, 70) : random(60, 150))) {
			flashTime = now;
			flashOn = !flashOn;

			if (flashOn) {
				// Flash: bright white with blue hint
				for (int i = 0; i < strip.numPixels(); i++) {
					uint8_t white = random(220, 255);
					uint8_t blue = random(200, 255);
					strip.setPixelColor(i, strip.Color(white, white, blue));
				}
				strip.show();
			}
			else {
				// Off between flashes
				for (int i = 0; i < strip.numPixels(); i++) {
					strip.setPixelColor(i, 0);
				}
				strip.show();

				flashCount--;
				if (flashCount == 0) {
					flashing = false;
					nextFlashDelay = random(500, 3000);  // faster storm repeat
					lastFlash = now;
				}
			}
		}
	}
}

void LEDManager::creepyCrawl() {
	static unsigned long lastUpdate = 0;
	static const uint8_t numCrawlers = 3;
	static int crawlerPos[numCrawlers] = { 0, 10, 20 };
	static int crawlerDir[numCrawlers] = { 1, -1, 1 };
	static uint8_t crawlerHue[numCrawlers] = { 120, 180, 90 };

	unsigned long now = millis();
	if (now - lastUpdate < 15) return;  // was 25ms → now ~66 FPS
	lastUpdate = now;

	// Fade the whole strip more aggressively for fast motion trails
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		r = r * 0.25;
		g = g * 0.25;
		b = b * 0.25;

		strip.setPixelColor(i, r, g, b);
	}

	// Move each crawler
	for (uint8_t i = 0; i < numCrawlers; i++) {
		crawlerPos[i] += crawlerDir[i];

		// Bounce off edges and choose new creepy hue
		if (crawlerPos[i] <= 0 || crawlerPos[i] >= strip.numPixels() - 1) {
			crawlerDir[i] *= -1;
			crawlerHue[i] = random(70, 170);  // sickly green/yellow
		}

		// Crawler main light
		uint32_t color = strip.ColorHSV(crawlerHue[i] * 256, 255, 255);
		strip.setPixelColor(crawlerPos[i], color);

		// Optional subtle trail behind
		int trailPos = crawlerPos[i] - crawlerDir[i];
		if (trailPos >= 0 && trailPos < strip.numPixels()) {
			uint32_t trailColor = strip.ColorHSV(crawlerHue[i] * 256, 255, 80);
			strip.setPixelColor(trailPos, trailColor);
		}
	}

	strip.show();
}

void LEDManager::zombieMarch() {
	static unsigned long lastUpdate = 0;
	static int marchPos = 0;
	unsigned long now = millis();

	if (now - lastUpdate < 40) return;  // was 100ms → now ~25 FPS
	lastUpdate = now;

	const uint16_t numPixels = strip.numPixels();

	// Fade strip to create trailing glow (dim only green channel)
	for (int i = 0; i < numPixels; i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		g = g * 0.85;  // slower fade than before (was 0.7)

		strip.setPixelColor(i, 0, g, 0);
	}

	// Bright marching zombies
	const int marchWidth = 4;
	for (int i = 0; i < marchWidth; i++) {
		int pos = (marchPos + i) % numPixels;
		uint8_t flicker = random(180, 255);  // brighter minimum
		strip.setPixelColor(pos, 0, flicker, 0);
	}

	marchPos = (marchPos + 1) % numPixels;

	strip.show();
}

void LEDManager::eyesInTheDark() {
	static unsigned long lastUpdate = 0;
	static unsigned long lastEyeSpawn = 0;
	static const uint16_t EYE_LIFESPAN = 300; // milliseconds eyes stay lit
	static const uint8_t MAX_EYES = 10;

	struct EyePair {
		int pos;
		uint32_t color;
		unsigned long spawnTime;
		bool active;
	};

	static EyePair eyes[MAX_EYES];

	unsigned long now = millis();

	// Clear strip to black every frame
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, 0);
	}

	// Occasionally spawn a new eye pair
	if (now - lastEyeSpawn > random(100, 400)) {
		for (int i = 0; i < MAX_EYES; i++) {
			if (!eyes[i].active) {
				int pos = random(0, strip.numPixels() - 1);  // leave room for 2 LEDs
				uint32_t color;

				// Pick a random eerie eye color
				switch (random(0, 3)) {
				case 0: color = strip.Color(255, 0, 0); break;   // red
				case 1: color = strip.Color(0, 255, 0); break;   // green
				case 2: color = strip.Color(255, 255, 255); break; // white
				}

				eyes[i] = { pos, color, now, true };
				lastEyeSpawn = now;
				break;
			}
		}
	}

	// Draw all active eyes
	for (int i = 0; i < MAX_EYES; i++) {
		if (eyes[i].active) {
			if (now - eyes[i].spawnTime < EYE_LIFESPAN) {
				strip.setPixelColor(eyes[i].pos, eyes[i].color);
				if (eyes[i].pos + 1 < strip.numPixels()) {
					strip.setPixelColor(eyes[i].pos + 1, eyes[i].color); // pair
				}
			}
			else {
				eyes[i].active = false;
			}
		}
	}

	strip.show();

}

#else

#include <EEPROM.h>

// EEPROM addresses | Look in system_config.h for avalable addresses.  Looks like currently 5-6 are next in line and  unused.
#define EEPROM_PATTERN_ADDR 5
#define EEPROM_BRIGHTNESS_ADDR 6

LEDManager::LEDManager(uint8_t pin, uint16_t numLeds)
	: strip(numLeds, pin, NEO_RGB + NEO_KHZ800),
	patternIndex(2),
	brightnessLevelIndex(4),  // Default to max brightness    
	baseHue(0),
	pulseStartTime(0),
	lastUpdate(0),
	settingsChanged(false),
	settingsChangeTime(0),
	EEPROM_SAVE_DELAY(10000) // 10 seconds
{}

void LEDManager::begin() {
	strip.begin();

	randomSeed(analogRead(A0));  // or any unused analog pin | Critical to keep from freezing/locking up | LED Patterns that need it: FireFlicker

	// Read from EEPROM
	uint8_t storedPattern = EEPROM.read(EEPROM_PATTERN_ADDR);
	uint8_t storedBrightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);

	// Validate and apply stored pattern
	if (storedPattern < NUM_PATTERNS) {
		patternIndex = storedPattern;
	}
	else {
		patternIndex = 2; // fallback to rainbow
		EEPROM.update(EEPROM_PATTERN_ADDR, patternIndex);
	}

	// Validate and apply stored brightness
	if (storedBrightness < NUM_BRIGHTNESS_LEVELS) {
		brightnessLevelIndex = storedBrightness;
	}
	else {
		brightnessLevelIndex = NUM_BRIGHTNESS_LEVELS - 1; // fallback to max
		EEPROM.update(EEPROM_BRIGHTNESS_ADDR, brightnessLevelIndex);
	}

	strip.setBrightness(brightnessLevels[brightnessLevelIndex]);
	strip.show();

	pulseStartTime = millis();
}

void LEDManager::update() {
	unsigned long now = millis();
	if (now - lastUpdate < 20) return;  // Limit update rate
	lastUpdate = now;

	switch (patternIndex) {
	case 0: solidWhite(); break;
	case 1: pulsingWhite(); break;
	case 2: rainbow(); break;
	case 3: rainbowChase(); break;
	case 4: fireFlicker(); break;
	case 5: plasma(); break;
	case 6: sparkle(); break;
	case 7: sparkleColorful(); break;
	case 8: auroraFade(); break;
	case 9: colorDrift(); break;
	case 10: spookyFlicker(); break;
	case 11: ghostPulse(); break;
	case 12: lightningStorm(); break;
	case 13: creepyCrawl(); break;
	case 14: zombieMarch(); break;
	case 15: eyesInTheDark(); break;
	}

	// Save settings to EEPROM if changed and 10s passed
	if (settingsChanged && now - settingsChangeTime >= EEPROM_SAVE_DELAY) {
		EEPROM.update(EEPROM_PATTERN_ADDR, patternIndex);
		EEPROM.update(EEPROM_BRIGHTNESS_ADDR, brightnessLevelIndex);
		settingsChanged = false;
	}
}

void LEDManager::nextPattern() {
	uint8_t newIndex = (patternIndex + 1) % NUM_PATTERNS;
	if (newIndex != patternIndex) {
		patternIndex = newIndex;
		settingsChanged = true;
		settingsChangeTime = millis();

		// Reset animation state for some patterns
		pulseStartTime = millis();
	}
}

void LEDManager::nextBrightness() {
	uint8_t newIndex = (brightnessLevelIndex + 1) % NUM_BRIGHTNESS_LEVELS;
	if (newIndex != brightnessLevelIndex) {
		brightnessLevelIndex = newIndex;
		strip.setBrightness(brightnessLevels[brightnessLevelIndex]);
		strip.show();
		settingsChanged = true;
		settingsChangeTime = millis();
	}
}

const char* LEDManager::getCurrentPatternName() const {
	if (patternIndex < NUM_PATTERNS) {
		return PATTERN_NAMES[patternIndex];
	}
	return "Unknown";
}

uint8_t LEDManager::getPatternIndex() const {
	return patternIndex;
}

uint8_t LEDManager::getBrightnessLevel() const {
	return brightnessLevelIndex;
}

void LEDManager::solidWhite() {
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(255, 255, 255));
	}
	strip.show();
}

void LEDManager::pulsingWhite() {
	float time = (millis() - pulseStartTime) / 1000.0;
	float speed = 0.2;
	float brightness = (sin(time * TWO_PI * speed) + 1.0) * 0.5 * 255;
	uint8_t b = (uint8_t)brightness;

	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(b, b, b));
	}
	strip.show();
}

void LEDManager::rainbow() {
	baseHue += 256;
	const uint8_t colorDensity = 4;

	for (int i = 0; i < strip.numPixels(); i++) {
		uint16_t pixelHue = baseHue + (i * 65536L * colorDensity / strip.numPixels());
		strip.setPixelColor(i, strip.ColorHSV(pixelHue, 255, 255));
	}
	strip.show();
}

void LEDManager::fireFlicker() {
	unsigned long now = millis();

	// Update every 50ms
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Flicker brightness between 150 and 255
		uint8_t flicker = random(150, 256);
		strip.setPixelColor(i, strip.Color(flicker, flicker / 3, 0)); // warm orange-yellow
	}
	strip.show();
}

void LEDManager::sparkle() {
	unsigned long now = millis();

	// Limit update rate (e.g. every 50ms)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	// Fade all pixels slightly
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		// Dim each color slightly
		r = r * 0.8;
		g = g * 0.8;
		b = b * 0.8;

		strip.setPixelColor(i, r, g, b);
	}

	// Light up a few random sparkles
	const uint8_t sparkleCount = strip.numPixels() / 20;  // ~5% of LEDs
	for (int i = 0; i < sparkleCount; i++) {
		int pixel = random(0, strip.numPixels());
		uint8_t brightness = random(150, 256);
		strip.setPixelColor(pixel, strip.Color(brightness, brightness, brightness)); // white sparkle
	}

	strip.show();
}

void LEDManager::sparkleColorful() {
	unsigned long now = millis();

	// Limit update rate (e.g. every 50ms)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	// Fade all pixels slightly
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		r = r * 0.8;
		g = g * 0.8;
		b = b * 0.8;

		strip.setPixelColor(i, r, g, b);
	}

	// Add random colorful sparkles
	const uint8_t sparkleCount = strip.numPixels() / 20;  // ~5% sparkle
	for (int i = 0; i < sparkleCount; i++) {
		int pixel = random(0, strip.numPixels());
		uint8_t hue = random(0, 256);
		uint32_t color = strip.ColorHSV(hue * 256, 255, 255);  // full sat/val
		strip.setPixelColor(pixel, color);
	}

	strip.show();
}

void LEDManager::rainbowChase() {
	unsigned long now = millis();

	// Update every 50ms
	static unsigned long lastUpdate = 0;
	static uint16_t chaseOffset = 0;  // Animation offset
	if (now - lastUpdate < 50) return;
	lastUpdate = now;

	chaseOffset++;  // Moves pattern forward

	const uint8_t stripeLength = 10;  // How many LEDs per cycle
	const uint8_t colorDensity = 4;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Create a moving pattern using offset
		int pos = (i + chaseOffset) % (stripeLength * 2);

		if (pos < stripeLength) {
			// Only light part of the pattern
			uint16_t hue = (i * 65536L * colorDensity / strip.numPixels()) + (chaseOffset * 256);
			strip.setPixelColor(i, strip.ColorHSV(hue, 255, 255));
		}
		else {
			// Leave other half dark
			strip.setPixelColor(i, 0);
		}
	}

	strip.show();
}

void LEDManager::plasma() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 30) return;
	lastUpdate = now;

	float speed = 0.0015;  // how fast the waves move
	float t = millis() * speed;

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();  // 0.0 to 1.0
		float wave = sin(2 * PI * (pos * 3.0 - t));  // 3 waves across strip

		// Brightness between 0 and 255
		uint8_t brightness = (wave + 1.0) * 0.5 * 255;

		// Shift hue over time
		uint16_t hue = ((uint16_t)(t * 2000) + i * 30) % 65536;

		uint32_t color = strip.ColorHSV(hue, 255, brightness);
		strip.setPixelColor(i, color);
	}

	strip.show();
}

void LEDManager::auroraFade() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;  // update every 50ms
	lastUpdate = now;

	float time = now / 10000.0;  // slow drift

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();

		// Create smooth brightness pulsing across the strip
		float wave = sin(2 * PI * (pos * 2.0 + time));  // 2 waves across strip

		uint8_t brightness = (wave + 1.0) * 0.5 * 255;

		// Use a drifting hue within a cool range (green/blue/purple)
		uint16_t baseHue = 16000;  // green-blue start
		uint16_t hueRange = 15000; // spread across cool tones
		uint16_t hue = baseHue + (uint16_t)(sin(time + pos * 2.0) * hueRange);

		strip.setPixelColor(i, strip.ColorHSV(hue, 180, brightness));  // softer saturation
	}

	strip.show();
}

void LEDManager::colorDrift() {
	unsigned long now = millis();

	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 50) return;  // update every 50ms
	lastUpdate = now;

	float time = now / 10000.0;  // slower = smoother

	// Slowly change hue over time (wraps automatically at 65535)
	uint16_t hue = (uint16_t)(sin(time * TWO_PI) * 32767 + 32768);

	// Apply this color to all pixels
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.ColorHSV(hue, 200, 255));
	}

	strip.show();
}

void LEDManager::spookyFlicker() {
	static unsigned long lastUpdate = 0;
	unsigned long now = millis();
	if (now - lastUpdate < 50) return; // Update ~20 FPS
	lastUpdate = now;

	for (int i = 0; i < strip.numPixels(); i++) {
		// Random warm flicker color
		uint8_t flicker = random(120, 255);
		uint8_t red = flicker;
		uint8_t green = flicker * random(30, 60) / 100;  // dimmer green
		uint8_t blue = random(0, 20);  // very subtle blue or none

		// Occasionally (very rarely), flash a ghostly blue-white (eerie effect)
		if (random(0, 1000) < 2) {  // ~0.2% chance per pixel per frame
			red = 200;
			green = 200;
			blue = 255;
		}

		strip.setPixelColor(i, red, green, blue);
	}

	strip.show();
}

void LEDManager::ghostPulse() {
	unsigned long now = millis();

	// Frame rate limiting (~40 FPS)
	static unsigned long lastUpdate = 0;
	if (now - lastUpdate < 25) return;
	lastUpdate = now;

	float time = now / 1000.0;  // seconds
	float speed = 0.15;         // slower pulse
	float pulse = (sin(time * TWO_PI * speed) + 1.0) * 0.5;  // 0.0 to 1.0

	for (int i = 0; i < strip.numPixels(); i++) {
		float pos = (float)i / strip.numPixels();

		// Slight hue shift across strip and over time
		uint16_t baseHue = 18000;  // cold blue
		uint16_t hueRange = 8000;  // into violet
		uint16_t hue = baseHue + (uint16_t)(sin(time + pos * 3.0) * hueRange);

		// Random ghost flash (very rare)
		bool ghostFlash = random(0, 2000) < 2;  // ~0.1% chance per pixel/frame

		uint8_t brightness = pulse * 180 + random(0, 20);  // flickery but soft

		if (ghostFlash) {
			// Spectral white-blue flash
			strip.setPixelColor(i, strip.Color(200, 200, 255));
		}
		else {
			// Soft blue/violet ghost glow
			uint32_t color = strip.ColorHSV(hue, 100, brightness);  // low sat
			strip.setPixelColor(i, color);
		}
	}

	strip.show();
}

void LEDManager::lightningStorm() {
	static unsigned long lastFlash = 0;
	static unsigned long nextFlashDelay = 0;
	static uint8_t flashCount = 0;
	static bool flashing = false;

	unsigned long now = millis();

	if (!flashing && now - lastFlash >= nextFlashDelay) {
		// Start a lightning sequence
		flashing = true;
		flashCount = random(2, 5);  // More flashes per storm
		lastFlash = now;
	}

	if (flashing) {
		static bool flashOn = false;
		static unsigned long flashTime = 0;

		// Flash durations — much faster and punchier
		if (now - flashTime > (flashOn ? random(30, 70) : random(60, 150))) {
			flashTime = now;
			flashOn = !flashOn;

			if (flashOn) {
				// Flash: bright white with blue hint
				for (int i = 0; i < strip.numPixels(); i++) {
					uint8_t white = random(220, 255);
					uint8_t blue = random(200, 255);
					strip.setPixelColor(i, strip.Color(white, white, blue));
				}
				strip.show();
			}
			else {
				// Off between flashes
				for (int i = 0; i < strip.numPixels(); i++) {
					strip.setPixelColor(i, 0);
				}
				strip.show();

				flashCount--;
				if (flashCount == 0) {
					flashing = false;
					nextFlashDelay = random(500, 3000);  // faster storm repeat
					lastFlash = now;
				}
			}
		}
	}
}

void LEDManager::creepyCrawl() {
	static unsigned long lastUpdate = 0;
	static const uint8_t numCrawlers = 3;
	static int crawlerPos[numCrawlers] = { 0, 10, 20 };
	static int crawlerDir[numCrawlers] = { 1, -1, 1 };
	static uint8_t crawlerHue[numCrawlers] = { 120, 180, 90 };

	unsigned long now = millis();
	if (now - lastUpdate < 15) return;  // was 25ms → now ~66 FPS
	lastUpdate = now;

	// Fade the whole strip more aggressively for fast motion trails
	for (int i = 0; i < strip.numPixels(); i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		r = r * 0.25;
		g = g * 0.25;
		b = b * 0.25;

		strip.setPixelColor(i, r, g, b);
	}

	// Move each crawler
	for (uint8_t i = 0; i < numCrawlers; i++) {
		crawlerPos[i] += crawlerDir[i];

		// Bounce off edges and choose new creepy hue
		if (crawlerPos[i] <= 0 || crawlerPos[i] >= strip.numPixels() - 1) {
			crawlerDir[i] *= -1;
			crawlerHue[i] = random(70, 170);  // sickly green/yellow
		}

		// Crawler main light
		uint32_t color = strip.ColorHSV(crawlerHue[i] * 256, 255, 255);
		strip.setPixelColor(crawlerPos[i], color);

		// Optional subtle trail behind
		int trailPos = crawlerPos[i] - crawlerDir[i];
		if (trailPos >= 0 && trailPos < strip.numPixels()) {
			uint32_t trailColor = strip.ColorHSV(crawlerHue[i] * 256, 255, 80);
			strip.setPixelColor(trailPos, trailColor);
		}
	}

	strip.show();
}

void LEDManager::zombieMarch() {
	static unsigned long lastUpdate = 0;
	static int marchPos = 0;
	unsigned long now = millis();

	if (now - lastUpdate < 40) return;  // was 100ms → now ~25 FPS
	lastUpdate = now;

	const uint16_t numPixels = strip.numPixels();

	// Fade strip to create trailing glow (dim only green channel)
	for (int i = 0; i < numPixels; i++) {
		uint32_t color = strip.getPixelColor(i);
		uint8_t r = (color >> 16) & 0xFF;
		uint8_t g = (color >> 8) & 0xFF;
		uint8_t b = color & 0xFF;

		g = g * 0.85;  // slower fade than before (was 0.7)

		strip.setPixelColor(i, 0, g, 0);
	}

	// Bright marching zombies
	const int marchWidth = 4;
	for (int i = 0; i < marchWidth; i++) {
		int pos = (marchPos + i) % numPixels;
		uint8_t flicker = random(180, 255);  // brighter minimum
		strip.setPixelColor(pos, 0, flicker, 0);
	}

	marchPos = (marchPos + 1) % numPixels;

	strip.show();
}

void LEDManager::eyesInTheDark() {
	static unsigned long lastUpdate = 0;
	static unsigned long lastEyeSpawn = 0;
	static const uint16_t EYE_LIFESPAN = 300; // milliseconds eyes stay lit
	static const uint8_t MAX_EYES = 10;

	struct EyePair {
		int pos;
		uint32_t color;
		unsigned long spawnTime;
		bool active;
	};

	static EyePair eyes[MAX_EYES];

	unsigned long now = millis();

	// Clear strip to black every frame
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, 0);
	}

	// Occasionally spawn a new eye pair
	if (now - lastEyeSpawn > random(100, 400)) {
		for (int i = 0; i < MAX_EYES; i++) {
			if (!eyes[i].active) {
				int pos = random(0, strip.numPixels() - 1);  // leave room for 2 LEDs
				uint32_t color;

				// Pick a random eerie eye color
				switch (random(0, 3)) {
				case 0: color = strip.Color(255, 0, 0); break;   // red
				case 1: color = strip.Color(0, 255, 0); break;   // green
				case 2: color = strip.Color(255, 255, 255); break; // white
				}

				eyes[i] = { pos, color, now, true };
				lastEyeSpawn = now;
				break;
			}
		}
	}

	// Draw all active eyes
	for (int i = 0; i < MAX_EYES; i++) {
		if (eyes[i].active) {
			if (now - eyes[i].spawnTime < EYE_LIFESPAN) {
				strip.setPixelColor(eyes[i].pos, eyes[i].color);
				if (eyes[i].pos + 1 < strip.numPixels()) {
					strip.setPixelColor(eyes[i].pos + 1, eyes[i].color); // pair
				}
			}
			else {
				eyes[i].active = false;
			}
		}
	}

	strip.show();
}

#endif // End ARDUINO NANO R4