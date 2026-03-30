#include "../../include/safety/thermal_security.h"
#if DISPLAY_TYPE_ST7796S
#include "../../include/display/view_manager_st7796s.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Adafruit_ST7796S.h>
//#include <Fonts/FreeSansBold18pt7b.h>  // A custom font
#include "../../include/utils/icons.h"
#include <cstring>
#if LED_MANAGER_ENABLED
#include "../../include/lights/LEDManager.h"
extern LEDManager ledManager;
#endif
// This is a faster option with HW SPI, but it requires using the hardware SPI pins (SCLK and MOSI) for the display, 
// which may limit pin availability for other components on the Arduino Nano. 
// If you have enough pins available and your display supports hardware SPI, 
// this is the recommended option for better performance.
Adafruit_ST7796S display(CS_PIN, DC_PIN, RST_PIN); 
// This is a very slow option with SW SPI, but it allows us to use the same pins for the display as for the other components, 
// which is necessary on the Arduino Nano with limited pins. For better performance, 
// consider using hardware SPI pins if your display supports it and if you have enough pins available.
// Adafruit_ST7796S display(CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN); 

static bool stringDisplayed = false;
// Static storage to remember last shown warning and whether it has been displayed.
static uint8_t animFrame = 0;
static uint32_t lastAnim = 0;
static float lastTemp = 0.0f;
static float lastHum = 0.0f;
static int checkGetControlMode = -1;
static bool isSystemOffLast = false;
static float lastFanSpeed = -1.0f;
static bool heaterOnState = false;
static int lastBrightness = -1;
static char lastLEDPattern[64] = "";
static char lastString[64] = "";
int HeightOnScreen = 0;       //  Create a variable to store the height of the icons, so we can adjust the Y alignment of text accordingly. This is calculated in drawInfo() when the icons are drawn, and used in all other screens to align text with the icons.
int PixelsBetweenRows = 5;     //  Create a variable to store the number of pixels between rows of text, so we can adjust the Y alignment of text accordingly. This is calculated in drawInfo() when the icons are drawn, and used in all other screens to align text with the icons.

ViewManager viewManager;

void ViewManager::begin(Adafruit_ST7796S& disp) {
	_display = &disp;
	_display->init(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, ST7796S_RGB);
	_display->invertDisplay(false); // Set to true if you want to invert the display colors
}

void ViewManager::draw() {
	static ViewID lastView = ViewID::RESET_SCREEN;
	if (_currentView != lastView) {
		_display->fillScreen(ST7796_BLACK);
		lastView = _currentView;
		stringDisplayed = false; // Reset string displayed flag when view changes
	}

	if (!_display) return;
	switch (_currentView) {
	case ViewID::SPLASH: drawSplash(); break;
	case ViewID::STANDBY: drawStandby(); break;
	case ViewID::INFO: drawInfo(); break;
	case ViewID::TEMP: drawTempConfig(); break;
	case ViewID::HUM: drawHumConfig(); break;
	case ViewID::MODE: drawModeConfig(); break;
	case ViewID::DRY_TIME: drawDryTimeConfig(); break;
	case ViewID::DRY_START: drawDryTimeStartConfig(); break;
	case ViewID::POWEROUT: drawPowerOutageMemory(); break;
	case ViewID::SCREEN_OFF: ScreenSaverOn(); break;
	case ViewID::SENSOR_ERROR: TempSensorError(); break;
	case ViewID::SCREEN_SAVER: drawScreenSaverSetting(); break;
	default: drawInfo(); break;
	}
}

void ViewManager::ScreenSaverOn() {
	_display->fillScreen(ST7796_BLACK);
}

void ViewManager::drawSplash() {
	int16_t bx, by;
	uint16_t w1, h1;

	_display->setTextSize(3);
	_display->setTextColor(ST7796_WHITE);

	_display->drawRGBBitmap((SCREEN_WIDTH / 2) - 120, SCREEN_HEIGHT / 12, Voxel_Logo_Color_Inv, 240, 240);  //  Draw a Voxel Logo icon at the end of the temp printout

	_display->setTextSize(2);
	_display->getTextBounds(str_code_by, 0, 0, &bx, &by, &w1, &h1);
	int16_t x1 = (SCREEN_WIDTH - w1) / 2;
	_display->setCursor(x1, SCREEN_HEIGHT / 1.548387f);  //changed for SH1107
	printProgmemString(_display, str_code_by);

	//_display->display();
}

//====================================================================================================
// Placeholder for info screen (can be expanded with more details)
// Current operation state is isSystemStandby or isSystemOff
// Main standby screen with temperature, humidity, and system status
//====================================================================================================
void ViewManager::drawStandby() {
	int16_t bx, by;
	uint16_t w1, h1;
	HeightOnScreen = 0;
	_display->setTextSize(7);
	_display->setTextColor(ST7796_WHITE);
	//_display->setFont(&FreeSansBold18pt7b);
	const char* msg;
	FanState fanState = getFanState();

	uint32_t currentTime = millis();
	if (currentTime - lastAnim > systemIntervals.animation) {
		animFrame = !animFrame;
		lastAnim = currentTime;
	}

	// Determine main status message (SYSTEM OFF, STANDBY, CYC, COOLING)
	if (stateManager.isSystemOff()) {
		msg = getProgmemString(str_system_off);
	}
	else if (fanState == FAN_COOLDOWN) {
		msg = getProgmemString(str_cooling);
	}
	else if (fanState == FAN_STANDBY_CYCLING_ON) {
		msg = getProgmemString(str_cycling);
	}
	else {
		msg = getProgmemString(str_standby);
	}

	// Show main status centered at the top, but only update if it changes to avoid unnecessary redraws
	_display->getTextBounds(msg, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
	// Retrieve the current warning string and print it only once,
	// and again only if its content changes.
	if (!stringDisplayed || strcmp(msg, lastString) != 0) {
		// store the new warning text
		strncpy(lastString, msg, sizeof(lastString) - 1);
		lastString[sizeof(lastString) - 1] = '\0';
		_display->fillRect(0, 0, SCREEN_WIDTH, h1, ST7796_BLACK);
		checkGetControlMode = -1; // reset control mode check text  to force redraw of control mode when main status changes
		lastTemp = 0.0f; // reset temp text to force redraw of temp when main status changes
		lastHum = 0.0f; // reset humidity text to force redraw of humidity when status changes
		lastFanSpeed = -1.0f; // reset fan speed text to force redraw of fan speed when status changes
		heaterOnState = true; // reset heater state text to force redraw of heater status when status changes
		lastBrightness = -1; // reset brightness text to force redraw of LED pattern when status changes
		lastLEDPattern[0] = '\0'; // reset LED pattern text to force redraw of LED pattern when status changes
		isSystemOffLast = !stateManager.isSystemOff(); // reset system off state text to force redraw of system off status when status changes
		printProgmemString(_display, msg);
		stringDisplayed = true;
	}

	HeightOnScreen += h1 + PixelsBetweenRows;

	// Show control mode or cooling status under the main status, but only when relevant
	_display->getTextBounds(str_cooling, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isSystemOff() && fanState == FAN_COOLDOWN) {
		// Show COOLING at bottom when in OFF state and cooldown
		_display->setTextSize(4);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
		_display->setTextColor(ST7796_CYAN);
		if (isSystemOffLast != stateManager.isSystemOff()) {
			isSystemOffLast = stateManager.isSystemOff();
			_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK);
			printProgmemString(_display, str_cooling);
		}
		HeightOnScreen += h1 + PixelsBetweenRows;
	}
	else if (stateManager.isSystemOff() && fanState != FAN_COOLDOWN) {
		_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK);
		HeightOnScreen += h1 + PixelsBetweenRows;
	}
	else if (!stateManager.isSystemOff()) {
		// Show control mode when not in OFF state
		_display->setTextColor(ST7796_WHITE);
		_display->setTextSize(4);
		_display->getTextBounds(str_auto_temp, 0, 0, &bx, &by, &w1, &h1);
		if (checkGetControlMode != stateManager.getControlMode()) {
			checkGetControlMode = stateManager.getControlMode();
			_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK);
			_display->setCursor(0, HeightOnScreen);
			_display->setTextSize(3);
			printProgmemString(_display, str_mode_control);
			_display->setTextSize(4);
			_display->setTextColor(checkGetControlMode == CONTROL_AUTO_TEMP || checkGetControlMode == CONTROL_AUTO_HUM ? ST7796_PINK : ST7796_YELLOW);
			switch (checkGetControlMode) {
			case CONTROL_AUTO_TEMP: printProgmemString(_display, str_auto_temp); break;
			case CONTROL_AUTO_HUM: printProgmemString(_display, str_auto_hum); break;
			case CONTROL_USER_TEMP: printProgmemString(_display, str_user_temp); break;
			case CONTROL_USER_HUM: printProgmemString(_display, str_user_hum); break;
			default: _display->setTextColor(ST7796_WHITE); break;
			}
			_display->setTextColor(ST7796_WHITE);
		}
		HeightOnScreen += h1 + PixelsBetweenRows;
	}

	// Show temperature and humidity readings with labels
	// Temperature and Humidity are only shown in STANDBY or OFF states, not when system is ON
	// This is to avoid cluttering the screen during active drying, 
	// and because these readings are most relevant in standby/off states for monitoring ambient conditions.
	// If you want to show them in ON state as well, simply remove the condition below.
	// 

	drawTemperature();
	drawHumidity();
	drawFanStatus();
	drawHeaterStatus();
#if LED_MANAGER_ENABLED
	drawLEDStatus();
#endif
	// End of updated code /////////////////////////////////////////////////////////////////////////
}

//====================================================================================================
// This function handles the INFO view, which shows the current operation status (SystemOn) (e.g., "Printing", "Drying", or warnings) 
// along with relevant icons and information. 
// It also includes an animation for warnings and ensures that warning messages are only updated
// when they change to minimize unnecessary redraws.
//====================================================================================================
void ViewManager::drawInfo() {
	extern StateManager stateManager;
	extern GyverPID heaterPID;
	int16_t bx, by;
	uint16_t w1, h1;
	uint32_t currentTime = millis();
	if (currentTime - lastAnim > systemIntervals.animation) {
		animFrame = !animFrame;
		lastAnim = currentTime;
	}

	// Static storage to remember last shown warning and whether it has been displayed.
	static const char* currentString = getProgmemString(str_warning);
	float displayTargetTemp = stateManager.getTargetTemp();
	_display->setTextSize(4);
	if (getThermalSecurityState() == THERMAL_PROTECTION && animFrame) {
		HeightOnScreen = 0;
		_display->drawBitmap(SCREEN_WIDTH / 2 - 50, 0, ThreeD_print_icon_100x100, 100, HeightOnScreen, ST7796_ORANGE);  //  Draw 3D printer icon at the top center of the screen
		HeightOnScreen += 100;
		_display->setTextSize(4);
		_display->getTextBounds(str_warning, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
		_display->setTextColor(ST7796_RED);
		_display->setTextSize(4);

		// Retrieve the current warning string and print it only once,
		// and again only if its content changes.
		if (!stringDisplayed || strcmp(currentString, lastString) != 0) {
			// store the new warning text
			strncpy(lastString, currentString, sizeof(lastString) - 1);
			lastString[sizeof(lastString) - 1] = '\0';
			_display->getTextBounds(currentString, 0, 0, &bx, &by, &w1, &h1);
			_display->fillRect(SCREEN_WIDTH / 32, HeightOnScreen - 2, SCREEN_WIDTH - 20, h1 + 1, ST7796_GREEN);
			printProgmemString(_display, currentString);
			stringDisplayed = true;
		}
	}
	else {
		// in DRY_MODE_BY_HUM, we show "Printing" with a 3D printer icon
		if (stateManager.getMode() == DRY_MODE_BY_HUM) {
			HeightOnScreen = 100;
			_display->drawBitmap(SCREEN_WIDTH / 2 - 50, 0, ThreeD_print_icon_100x100, 100, HeightOnScreen, ST7796_ORANGE);  //  Draw 3D printer icon at the top center of the screen
			_display->setTextSize(4);
			currentString = getProgmemString(str_printing);
			_display->getTextBounds(currentString, 0, 0, &bx, &by, &w1, &h1);
			HeightOnScreen += PixelsBetweenRows;
			if (!stringDisplayed || strcmp(currentString, lastString) != 0) {
				// store the new warning text
				strncpy(lastString, currentString, sizeof(lastString) - 1);
				lastString[sizeof(lastString) - 1] = '\0';
				_display->fillRect(SCREEN_WIDTH / 32, HeightOnScreen - 2, SCREEN_WIDTH - 20, h1 + 1, ST7796_GREEN);
				_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
				_display->setTextColor(ST7796_BLUE);
				printProgmemString(_display, currentString); // PRINTING text
				stringDisplayed = true;
			}
		}
		// In DRY_MODE_BY_TIME, we show "Drying" with a filament icon instead of "Printing"
		else {
			HeightOnScreen = 100;
			_display->drawBitmap(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 96, filament_icon_100x100, 100, HeightOnScreen, ST7796_MAGENTA);  //  Draw 3D printer icon at the top center of the screen
			_display->setTextSize(4);
			currentString = getProgmemString(str_drying);
			_display->getTextBounds(currentString, 0, 0, &bx, &by, &w1, &h1);
			HeightOnScreen += PixelsBetweenRows;
			if (!stringDisplayed || strcmp(currentString, lastString) != 0) {
				// store the new warning text
				strncpy(lastString, currentString, sizeof(lastString) - 1);
				lastString[sizeof(lastString) - 1] = '\0';
				_display->fillRect(SCREEN_WIDTH / 32, HeightOnScreen - 2, SCREEN_WIDTH - 20, h1 + 1, ST7796_GREEN);
				_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
				_display->setTextColor(ST7796_BLUE);
				printProgmemString(_display, currentString); // DRYING text
				stringDisplayed = true;
			}
		}

		// Display temperature and humidity with labels
		HeightOnScreen += h1 + PixelsBetweenRows;
		_display->setCursor(SCREEN_WIDTH / 5.3333f, HeightOnScreen);
		_display->setTextColor(ST7796_WHITE);
		_display->setTextSize(3);
		_display->getTextBounds(str_greater, 0, 0, &bx, &by, &w1, &h1);
		if (sensorManager.getTemperature() != lastTemp) {
			lastTemp = sensorManager.getTemperature();
			_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK);
			_display->print(sensorManager.getTemperature(), 1);
			printProgmemString(_display, getTemperatureUnit());
			printProgmemString(_display, str_greater);
			_display->print(displayTargetTemp, 0);
			printProgmemString(_display, getTemperatureUnit());
		}
		HeightOnScreen += h1 + PixelsBetweenRows;
		_display->setCursor(SCREEN_WIDTH / 5.3333f, HeightOnScreen);
		if (sensorManager.getHumidity() != lastHum) {
			lastHum = sensorManager.getHumidity();
			_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK);
			_display->print(sensorManager.getHumidity(), 1);
			printProgmemString(_display, str_percent);
			if (stateManager.getMode() == DRY_MODE_BY_HUM) printProgmemString(_display, str_greater), _display->print(stateManager.getTargetHumidity()), printProgmemString(_display, str_percent_sign);
		}
		HeightOnScreen += h1 + PixelsBetweenRows;


		// Display heater status and fan status with labels
		// inside a white box when active, with flashing effect
		_display->setFont();
		_display->setTextSize(2);
		ThermalSecurityState thermalState = getThermalSecurityState();
		FanState fanState = getFanState();
		bool fanActive = (thermalState == THERMAL_COOLDOWN) || (fanState == FAN_COOLDOWN) || isFanOn();
		_display->getTextBounds(str_cycling, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
		_display->fillRect(((SCREEN_WIDTH / 2) - (w1 / 2) - 5), HeightOnScreen - 1, w1 + 10, h1 + 2, ST7796_WHITE);
		_display->setTextColor(ST7796_BLACK);
		printProgmemString(_display, (thermalState == THERMAL_COOLDOWN) ? str_balance : (fanState == FAN_COOLDOWN) ? str_cooling : isFanOn() ? str_cycling : str_fan_off);
		_display->setTextColor(ST7796_WHITE);
		HeightOnScreen += h1 + PixelsBetweenRows;

		if (stateManager.getMode() == DRY_MODE_BY_TIME) {
			_display->setTextColor(ST7796_WHITE);
			uint32_t remainingSeconds = stateManager.getRemainingDryTime();
			uint16_t hours = remainingSeconds / SECONDS_PER_HOUR;
			uint16_t minutes = (remainingSeconds % SECONDS_PER_HOUR) / 60;
			static uint16_t minutes_temp = -1;

			// Combined buffer for label + formatted time "str_eta_label HH:MM"
			char timeBuf[64];

			// Build a single line string with the PROGMEM label and formatted time.
			// getProgmemString returns a regular const char* here, so it's safe to use in snprintf.
			snprintf(timeBuf, sizeof(timeBuf), "%s %02u:%02u", getProgmemString(str_eta_label), hours, minutes);
			_display->setTextSize(4);
			//_display->setFont(&FreeSansBold18pt7b);
			_display->getTextBounds(timeBuf, 0, 0, &bx, &by, &w1, &h1);
			if (minutes != minutes_temp) {
				minutes_temp = minutes;
				_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK); // Clear previous time
			}
			_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), HeightOnScreen);
			_display->print(timeBuf);
			_display->setFont();
			HeightOnScreen += h1 + PixelsBetweenRows;
		}

		//====================================================================================================
		// Show Fan, Temp and LED status with big text and icons
		// common for PRINTING and DRYING modes
		//====================================================================================================

		drawFanStatus();
		drawHeaterStatus();
#if LED_MANAGER_ENABLED
		drawLEDStatus();
#endif
	}
}

//============================================================================================================
// This function initializes the layout for sub-screens (like settings screens) with a title and prepares
// the area for displaying the setting value.It also handles the text color based on whether the user
// is in edit mode or not, and includes a flashing effect for the editable value.
//============================================================================================================
void ViewManager::StartSubScreens(const char* title, uint8_t textSize) {
	stringDisplayed = false;
	int16_t bx, by;
	uint16_t w1, h1;
	HeightOnScreen = SCREEN_HEIGHT / 24;
	_display->setTextSize(textSize);
	_display->setTextColor(ST7796_WHITE);
	_display->getTextBounds(title, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	printProgmemString(_display, title);
	HeightOnScreen += h1 + (SCREEN_HEIGHT / 24);
}

void ViewManager::SecondTitleRow(const char* secondTitle) {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->getTextBounds(secondTitle, 0, 0, &bx, &by, &w1, &h1);
	HeightOnScreen -= h1 + (SCREEN_HEIGHT / 24);
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	printProgmemString(_display, secondTitle);
	HeightOnScreen += h1 + (SCREEN_HEIGHT / 24);
}

//============================================================================================================
// This function displays the "Change and Save" instruction at the bottom of the screen, prompting
// the user to save their changes after editing a setting. It is used across multiple configuration screens
// to maintain consistency in user instructions.
//============================================================================================================
void ViewManager::SaveEnterText() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->setTextSize(3);
	_display->getTextBounds(str_change_save, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	HeightOnScreen += h1 + PixelsBetweenRows;
	printProgmemString(_display, str_change_save);
}
//============================================================================================================
// This function draws the temperature configuration screen, allowing the user to set the target temperature.
// It includes a flashing effect on the temperature value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawTempConfig() {
	stringDisplayed = false;
	extern StateManager stateManager;
	static uint8_t lastTargetTemp = 0;
	static bool isEditingLast = false;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_set_temp, 4);
	_display->setTextSize(8);
	// Convert numeric target temperature to string
	char tempBuf[8];
	snprintf(tempBuf, sizeof(tempBuf), "%u C", stateManager.getTargetTemp());
	_display->getTextBounds(tempBuf, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastTargetTemp = 0; // reset last target temp to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getTargetTemp() != lastTargetTemp) {
			lastTargetTemp = stateManager.getTargetTemp();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getTargetTemp() != lastTargetTemp) {
			lastTargetTemp = stateManager.getTargetTemp();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	_display->print(tempBuf);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 32), HeightOnScreen, temp_icon_64x64, 64, 64, ST7796_RED);  //  Draw a thermometer icon at the end of the temp printout
}

//============================================================================================================
// This function draws the humidity configuration screen, allowing the user to set the target humidity for humidity-based drying mode.
// It includes a flashing effect on the humidity value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawHumConfig() {
	extern StateManager stateManager;
	static float lastTargetHum = 0.0f;
	static bool isEditingLast = false;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_set_humidity, 4);
	_display->setTextSize(8);
	// Convert numeric target Humidity to string
	char HumidBuf[8];
	snprintf(HumidBuf, sizeof(HumidBuf), "%u%%", stateManager.getTargetHumidity());
	_display->getTextBounds(HumidBuf, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastTargetHum = 0.0f; // reset last target humidity to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getTargetHumidity() != lastTargetHum) {
			lastTargetHum = stateManager.getTargetHumidity();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getTargetHumidity() != lastTargetHum) {
			lastTargetHum = stateManager.getTargetHumidity();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	_display->print(HumidBuf);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 32), HeightOnScreen, humidity_icon_64x64, 64, 64, ST7796_BLUE);  //  Draw droplets icon at the end of the humidity printout
}

//============================================================================================================
// This function draws the screen saver configuration screen, allowing the user to set the time until screen goes blank.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawScreenSaverSetting() {
	extern StateManager stateManager;
	static int8_t lastTimeout = -1;
	static bool isEditingLast = false;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_screen_saver, 4);
	_display->setTextSize(8);
	// Convert numeric target Humidity to string
	char timeoutBuf[18];
	sprintf(timeoutBuf, "%u min", stateManager.getScreenTimeout());
	if (stateManager.getScreenTimeout() == 0)
		strcpy(timeoutBuf, getProgmemString(str_screen_saver_never));
	_display->getTextBounds(timeoutBuf, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastTimeout = -1; // reset last mode to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getScreenTimeout() != lastTimeout) {
			lastTimeout = stateManager.getScreenTimeout();
			_display->fillRect(0, HeightOnScreen - 10, SCREEN_WIDTH, h1 + 12, ST7796_BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getScreenTimeout() != lastTimeout) {
			lastTimeout = stateManager.getScreenTimeout();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	_display->print(timeoutBuf);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 50), HeightOnScreen, screen_saver_100x93, 100, 93, ST7796_YELLOW);  //  Draw droplets icon at the end of the humidity printout
}
	
//============================================================================================================
// This function draws the drying time configuration screen, allowing the user to set the duration for time-based drying mode.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawDryTimeConfig() {
	extern StateManager stateManager;
	static uint32_t lastDuration = 0;
	static bool isEditingLast = false;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_dry_mode_time, 4);
	_display->setTextSize(8);

	uint32_t duration = stateManager.getDryDuration();
	uint16_t hours = duration / 60;
	uint16_t minutes = duration % 60;

	char timeBuf[6];
	sprintf(timeBuf, "%02u:%02u", hours, minutes);
	_display->getTextBounds(timeBuf, 0, 0, &bx, &by, &w1, &h1);

	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastDuration = 0; // reset last duration to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getDryDuration() != lastDuration) {
			lastDuration = stateManager.getDryDuration();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getDryDuration() != lastDuration) {
			lastDuration = stateManager.getDryDuration();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	_display->print(timeBuf);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 50), HeightOnScreen, clock_icon_100x100, 100, 100, ST7796_WHITE);  //  Draw a filament icon at the end of the time printout
}


//===========================================================================================================
// This function sets the time for when the system starts the Dry time again after it has been turned off, 
// allowing the user to configure the delay before the system can be used again after being turned off.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//=============================================================================================================

void ViewManager::drawDryTimeStartConfig() {
	extern StateManager stateManager;
	static uint32_t lastStartTime = 0;
	static bool isEditingLast = false;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_dry_time_start, 3);
	_display->setTextSize(8);

	uint32_t startTimer = stateManager.getDryStartTimer();
	uint16_t hours = startTimer / 60;
	uint16_t minutes = startTimer % 60;

	char timerBuf[6];
	sprintf(timerBuf, "%02u:%02u", hours, minutes);
	_display->getTextBounds(timerBuf, 0, 0, &bx, &by, &w1, &h1);

	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastStartTime = -1; // reset last start time to force redraw when switching between edit and non-edit mode
	}

	if (stateManager.isEditing()) {
		if (stateManager.getDryStartTimer() != lastStartTime) {
			lastStartTime = stateManager.getDryStartTimer();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getDryStartTimer() != lastStartTime) {
			lastStartTime = stateManager.getDryStartTimer();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_BLACK);
		}				
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	_display->print(timerBuf);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 50), HeightOnScreen, timer_icon_100x112, 100, 112, ST7796_WHITE);  //  Draw a filament icon at the end of the time printout
}

//============================================================================================================
// This function draws the power outage memory screen, which informs the user about what happens after 
// a power outage and how the system retains settings and resumes operation. It includes relevant icons and instructions.
//============================================================================================================
void ViewManager::drawPowerOutageMemory() {
	extern StateManager stateManager;
	static bool isEditingLast = false;
	static int lastMode = -1;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_power_loss_memory_one, 4);
	SecondTitleRow(str_power_loss_memory_two);
	_display->setTextSize(5);
	_display->getTextBounds(stateManager.isPowerLossMemory() == false ? str_power_off : str_last_mode, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastMode = -1; // reset last mode to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.isPowerLossMemory() != lastMode) {
			lastMode = stateManager.isPowerLossMemory();
			_display->fillRect(0, HeightOnScreen - 10, SCREEN_WIDTH, h1 + 12, ST7796_BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.isPowerLossMemory() != lastMode) {
			lastMode = stateManager.isPowerLossMemory();
			_display->fillRect(0, HeightOnScreen - 10, SCREEN_WIDTH, h1 + 12, ST7796_BLACK);
		}
	}
 	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	printProgmemString(_display, stateManager.isPowerLossMemory() == false ? str_power_off : str_last_mode		);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawRGBBitmap((SCREEN_WIDTH / 2) - 50, HeightOnScreen, PowerOutageIcon_100x100_color, 100, 100);  // Draw Power outage icon with color support
}

//============================================================================================================
// This function draws the mode configuration screen, allowing the user to switch between humidity-based and time-based drying modes.
// It includes a flashing effect on the selected mode when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawModeConfig() {
	static bool isEditingLast = false;
	static int lastMode = -1;
	int16_t bx, by;
	uint16_t w1, h1;
	StartSubScreens(str_operation, 4);
	SecondTitleRow(str_mode);
	_display->setTextSize(5);
	_display->getTextBounds(stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastMode = -1; // reset last mode to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getMode() != lastMode) {
			lastMode = stateManager.getMode();
			_display->fillRect(0, HeightOnScreen - 10, SCREEN_WIDTH, h1 + 12, ST7796_BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 10, HeightOnScreen - 10, w1 + 20, h1 + 12, ST7796_WHITE);
		}
	}
	else {
		if (stateManager.getMode() != lastMode) {
			lastMode = stateManager.getMode();
			_display->fillRect(0, HeightOnScreen - 10, SCREEN_WIDTH, h1 + 12, ST7796_BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? ST7796_BLACK : ST7796_WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), HeightOnScreen);
	printProgmemString(_display, stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry);
	_display->setTextColor(ST7796_WHITE);
	HeightOnScreen += h1 + PixelsBetweenRows;
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 50), HeightOnScreen, modeicon_100x100, 100, 100, ST7796_YELLOW);  //  Draw mode icon at the end of the mode printout
}

void ViewManager::TempSensorError() {
	static bool blink = false;
	_display->fillScreen(ST7796_BLACK);
	_display->setTextSize(2);
	_display->setTextColor(ST7796_WHITE);
	_display->setFont();  // Use default font
	int16_t bx, by;
	uint16_t w1, h1, w2, h2, w3;
	const char* line1 = "No Temperature";
	const char* line2 = "sensor attached";
	const char* line3 = "No Humidity";
	const uint16_t spacing = 8;  // vertical spacing between lines in pixels

	_display->getTextBounds(line2, 0, 0, &bx, &by, &w2, &h2);
	if (blink) {
		// Measure both lines
		_display->getTextBounds(line1, 0, 0, &bx, &by, &w1, &h1);
	}
	else {
		// Measure both lines
		_display->getTextBounds(line3, 0, 0, &bx, &by, &w3, &h1);
	}

	uint16_t totalHeight = h1 + h2 + spacing;
	// Baseline Y for the first line so that the two lines are vertically centered
	int16_t firstBaselineY = (SCREEN_HEIGHT - totalHeight) / 2 + h1;

	// Center X for each line
	int16_t x1 = (SCREEN_WIDTH - w1) / 2;
	int16_t x2 = (SCREEN_WIDTH - w2) / 2;
	int16_t x3 = (SCREEN_WIDTH - w3) / 2;

	// Compute a rectangle that covers both lines and erase only that area
	int16_t rectX1 = min(x1, x2);
	int16_t rectX2 = min(x3, x2);
	uint16_t rectW1 = max(w1, w2);
	uint16_t rectW2 = max(w3, w2);
	int16_t rectY = firstBaselineY;  // top Y of first line
	uint16_t rectH = totalHeight;


	if (blink) {
		// Erase only the text area
		_display->fillRect(rectX2, rectY, rectW2, rectH, ST7796_BLACK);
		_display->setTextColor(ST7796_RED);
		// Print centered lines
		_display->setCursor(x1, firstBaselineY);
		_display->println(line1);
		_display->setCursor(x2, firstBaselineY + h1 + spacing);
		_display->println(line2);
	}
	else {
		// Erase only the text area
		_display->fillRect(rectX1, rectY, rectW1, rectH, ST7796_WHITE);
		_display->setTextColor(ST7796_BLACK);
		// Print centered lines
		_display->setCursor(x3, firstBaselineY);
		_display->println(line3);
		_display->setCursor(x2, firstBaselineY + h1 + spacing);
		_display->println(line2);
	}
	blink = !blink;
}

//============================================================================================================
// This function displays the current temperature readings on the screen, along with corresponding icons.
// It includes a flashing effect when the temperature changes to draw attention to the updated reading.
//============================================================================================================
void ViewManager::drawTemperature() {
	int16_t bx, by;
	uint16_t w1, h1;
	static uint16_t tempHeightOnScreen = HeightOnScreen;
	_display->setTextSize(3);  // Set small text for the TEMPERATURE Label
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for TEMPERATURE Label
	_display->setTextColor(ST7796_RED);
	_display->getTextBounds(str_temp_label, 0, 0, &bx, &by, &w1, &h1);
	if (tempHeightOnScreen != HeightOnScreen) {
		_display->fillRect(0, tempHeightOnScreen - 5, SCREEN_WIDTH, ((h1 * 2) + (PixelsBetweenRows * 3)), ST7796_BLACK);
		tempHeightOnScreen = HeightOnScreen; // Initialize tempHeightOnScreen on first call
	}
	printProgmemString(_display, str_temp_label);  //  Set the TEMPERATURE Label up on Screen
	_display->setTextColor(ST7796_WHITE);
	_display->drawBitmap(200, HeightOnScreen - 4, temp_icon_64x64, 64, 64, ST7796_RED);  //  Draw a thermometer icon at the end of the temp printout
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for Temperature Value
	_display->setTextSize(4);  // Set the font bigger for the Temp Reading

	// Read temperature and format into a string with 2 decimal places
	float currentTemp = sensorManager.getTemperature();
	char TempBuffer[16];
	snprintf(TempBuffer, sizeof(TempBuffer), "%.2f C", currentTemp);
	// Measure formatted text bounds
	_display->getTextBounds(TempBuffer, 0, 0, &bx, &by, &w1, &h1);
	if (currentTemp != lastTemp) {
		lastTemp = currentTemp;
		// Clear area large enough to accommodate temperature text
		_display->fillRect(0, HeightOnScreen, max((int)w1, (int)(SCREEN_WIDTH / 3)), h1, ST7796_BLACK);
		_display->setCursor(0, HeightOnScreen);
		_display->print(TempBuffer);  // Print formatted temperature string
	}
	HeightOnScreen += h1 + (PixelsBetweenRows * 2);
	//	printProgmemString(_display, str_space);  // Put a space between the temp numbers and The temp Unit        
	//	_display->drawCircle(139, HeightOnScreen, 5, ST7796_WHITE);  //  Draw the Degrees symbol
	//	_display->setCursor(146, HeightOnScreen); // set cursor to vertical alignment for Temperature Value
	//	printProgmemString(_display, getTemperatureUnit()); //  Put the temp unit (C or F) on the screen
}

//============================================================================================================
// This function displays the current humidity readings on the screen, along with corresponding icons.
// It includes a flashing effect when the humidity changes to draw attention to the updated reading.
//============================================================================================================
void ViewManager::drawHumidity() {
	int16_t bx, by;
	uint16_t w1, h1;
	static uint16_t humHeightOnScreen = HeightOnScreen;
	_display->setTextSize(3);  // Set the font smaller for the Humidity Label
	_display->getTextBounds(str_humidity_label, 0, 0, &bx, &by, &w1, &h1);
	if (humHeightOnScreen != HeightOnScreen) {
		_display->fillRect(0, humHeightOnScreen - 5, SCREEN_WIDTH, ((h1 * 2) + (PixelsBetweenRows * 3)), ST7796_BLACK);
		humHeightOnScreen = HeightOnScreen; // Initialize humHeightOnScreen on first call
	}
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for Humidy Label
	_display->setTextColor(ST7796_BLUE);
	printProgmemString(_display, str_humidity_label);  //Set the Humidity Label up on Screen
	_display->drawBitmap(160, HeightOnScreen - 4, humidity_icon_64x64, 64, 64, ST7796_BLUE);  //  Draw droplets icon at the end of the humidity printout
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for Temperature Value
	_display->setTextSize(4);  // Set the font bigger for the Humidity Reading

	// Read humidity and format into a string with 2 decimal places
	float currentHum = sensorManager.getHumidity();
	char HumBuffer[16];
	snprintf(HumBuffer, sizeof(HumBuffer), "%.2f%%", currentHum);
	// Measure formatted text bounds
	_display->getTextBounds(HumBuffer, 0, 0, &bx, &by, &w1, &h1);

	if (sensorManager.getHumidity() != lastHum) {
		lastHum = sensorManager.getHumidity();
		_display->fillRect(0, HeightOnScreen, max((int)w1, (int)(SCREEN_WIDTH / 3)), h1, ST7796_BLACK);
		_display->setTextColor(ST7796_WHITE);
		_display->print(HumBuffer);  // Print formatted temperature string
	}
	HeightOnScreen += h1 + (PixelsBetweenRows * 2);
}

void ViewManager::drawFanStatus() {
	int16_t bx, by;
	uint16_t w1, h1;
	static uint16_t fanHeightOnScreen = HeightOnScreen;
	static bool FansOnState = false;
	// Show Fan speed/status with label and icon, with flashing effect when fan is on or cooling
	_display->setTextSize(3);
	_display->getTextBounds(str_fan_format, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for Humidy Label
	_display->setTextColor(ST7796_MAGENTA);
	printProgmemString(_display, str_fan_format);

	if (fanHeightOnScreen != HeightOnScreen) {
		_display->fillRect(0, fanHeightOnScreen - 5, SCREEN_WIDTH, ((h1 * 2) + (PixelsBetweenRows * 3)), ST7796_BLACK);
		fanHeightOnScreen = HeightOnScreen; // Initialize fanHeightOnScreen on first call
	}
	if (getCurrentFanSpeed() > 0.0f) {
		FansOnState = true;
		_display->fillRect(180, HeightOnScreen - 4, 64, 64, ST7796_BLACK);
		_display->drawBitmap(180, HeightOnScreen - 4, animFrame ? fan_run1_64x64 : fan_run2_64x64, 64, 63, ST7796_MAGENTA);  //  Draw droplets icon at the end of the humidity printout
	}
	else {
		if (FansOnState) {
			FansOnState = false;
			_display->fillRect(180, HeightOnScreen - 4, 64, 64, ST7796_BLACK);
		}
		_display->drawBitmap(180, HeightOnScreen - 4, fan_still_64x64, 64, 63, ST7796_MAGENTA);  //  Draw droplets icon at the end of the humidity printout
	}
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for Temperature Value
	_display->setTextSize(4);  // Set the font bigger for the Humidity Reading

	// Read fan speed and format into a string with 0 decimal places
	float currentfan = (getCurrentFanSpeed() * 100.0f / 255.0f);
	char FanBuffer[16];
	snprintf(FanBuffer, sizeof(FanBuffer), "%u%%", (unsigned int)currentfan);
	_display->getTextBounds(str_fan_off, 0, 0, &bx, &by, &w1, &h1);

	if (getCurrentFanSpeed() > 0.0f) {
		if (getCurrentFanSpeed() != lastFanSpeed) {
			lastFanSpeed = getCurrentFanSpeed();
			_display->fillRect(0, HeightOnScreen, w1, h1, ST7796_BLACK);
		}
		_display->setTextColor(ST7796_WHITE);
		_display->print(FanBuffer);  // Print formatted temperature string
	}
	else {
		_display->setTextColor(ST7796_MAGENTA);
		if (getCurrentFanSpeed() != lastFanSpeed) {
			lastFanSpeed = getCurrentFanSpeed();
			_display->fillRect(0, HeightOnScreen, w1, h1, ST7796_BLACK);
			printProgmemString(_display, str_fan_off);
		}
	}
	HeightOnScreen += h1 + (PixelsBetweenRows * 2);
}

void ViewManager::drawHeaterStatus() {
	int16_t bx, by;
	uint16_t w1, h1;
	static uint16_t heatHeightOnScreen = HeightOnScreen;
	_display->setTextSize(4);

	// Show heater status with label and icon, with flashing effect when heater is on
	if (heatHeightOnScreen != HeightOnScreen) {
		_display->fillRect(0, heatHeightOnScreen - 5, SCREEN_WIDTH, ((h1 * 2) + (PixelsBetweenRows * 3)), ST7796_BLACK);
		heatHeightOnScreen = HeightOnScreen; // Initialize heatHeightOnScreen on first call
	}
	_display->setTextColor(stateManager.isHeaterOn() && animFrame ? ST7796_YELLOW : ST7796_RED);
	_display->getTextBounds(str_heat_off, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(0, heatHeightOnScreen + (h1 / 2));


	if (stateManager.isHeaterOn()) {
		heaterOnState = true;
		char heaterBuf[10];
		sprintf(heaterBuf, getProgmemString(str_heat_format), (int)(controlSystem.heaterPower * 100.0f / 255.0f));
		_display->getTextBounds(heaterBuf, 0, 0, &bx, &by, &w1, &h1);
		_display->fillRect(0, heatHeightOnScreen + (h1 / 2), w1, h1, stateManager.isHeaterOn() && animFrame ? ST7796_RED : ST7796_ORANGE);
		_display->print(heaterBuf);
		_display->fillRect(225, heatHeightOnScreen, 85, 68, ST7796_BLACK);
		_display->drawBitmap(225, heatHeightOnScreen, animFrame ? heating_icon_85x68 : heating_icon_mirror_85x68, 85, 68, animFrame ? ST7796_ORANGE : ST7796_RED);  //  Draw a heater icon at the end of the temp printout, with flashing effect when heater is on
	}
	else {
		_display->setTextColor(ST7796_CYAN);
		if (heaterOnState) {
			heaterOnState = false;
			_display->fillRect(0, heatHeightOnScreen + (h1 / 2), SCREEN_WIDTH, h1, ST7796_BLACK);
			_display->fillRect(225, heatHeightOnScreen, 85, 68, ST7796_BLACK);
			_display->drawBitmap(225, heatHeightOnScreen, heating_icon_85x68, 85, 68, ST7796_YELLOW);  //  Draw a heater icon at the end of the temp printout, with flashing effect when heater is on
		}
		printProgmemString(_display, str_heat_off);
	}
	HeightOnScreen += h1 + (PixelsBetweenRows * 2);
}

#if LED_MANAGER_ENABLED
void ViewManager::drawLEDStatus() {
	// OLED Screen Led Pattern Label : Scrolling : TextSize = 2  
	// Show Name of LED show that is playing - with scrolling if too long
	int16_t bx, by;
	uint16_t w1, h1;
	static uint16_t ledHeightOnScreen = HeightOnScreen;
	static bool heightInitialized = false;
	_display->setTextSize(3);
	_display->getTextBounds(str_led_pattern_label, 0, 0, &bx, &by, &w1, &h1);
	if (ledHeightOnScreen != HeightOnScreen) {
		_display->fillRect(0, ledHeightOnScreen - 5, SCREEN_WIDTH, ((h1 * 2) + (PixelsBetweenRows * 3)), ST7796_BLACK);
		ledHeightOnScreen = HeightOnScreen; // Initialize ledHeightOnScreen on first call
		heightInitialized = false;
	}
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for LED Label
	_display->setTextColor(ST7796_GREEN);
	printProgmemString(_display, str_led_pattern_label);  //Set the LED Label up on Screen
	_display->drawBitmap(220, HeightOnScreen, led_icon_51x63, 51, 63, ST7796_GREEN);  //  Draw a LED icon at the end of the temp printout
	HeightOnScreen += h1 + PixelsBetweenRows;

	_display->setTextSize(2);
	_display->setTextColor(ST7796_WHITE);
	_display->setCursor(0, HeightOnScreen); // set cursor to vertical alignment for LED Label
	_display->getTextBounds(ledManager.getCurrentPatternName(), 0, 0, &bx, &by, &w1, &h1);

	if (!stringDisplayed || strcmp(ledManager.getCurrentPatternName(), lastLEDPattern) != 0) {
		// store the new warning text
		strncpy(lastLEDPattern, ledManager.getCurrentPatternName(), sizeof(lastLEDPattern) - 1);
		lastLEDPattern[sizeof(lastLEDPattern) - 1] = '\0';
		_display->fillRect(0, HeightOnScreen, w1, h1 + 2, ST7796_BLACK); // Clear area for scrolling text
		stringDisplayed = true;
	}
	_display->print(ledManager.getCurrentPatternName());
	HeightOnScreen += h1 + PixelsBetweenRows;
	if (!heightInitialized || lastBrightness != ledManager.getBrightnessLevel()) {
		heightInitialized = true;
		lastBrightness = ledManager.getBrightnessLevel();
		_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, ST7796_BLACK); // Clear area for scrolling text
	}
	_display->setCursor(0, HeightOnScreen);
	printProgmemString(_display, str_brightness_level);  //Set the Brightness Label up on Screen
	printProgmemString(_display, str_space);  // Put a space between the Brightness and the level number
	_display->print(ledManager.getBrightnessLevel());
}
#endif	// LED_MANAGER_ENABLED

#endif	// DISPLAY_TYPE_ST7796S