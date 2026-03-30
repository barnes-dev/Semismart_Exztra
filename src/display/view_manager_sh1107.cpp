#include "../../include/safety/thermal_security.h"
#if DISPLAY_TYPE_SH110X
#include "../../include/display/view_manager_sh1107.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include "../../include/utils/icons.h"
#if LED_MANAGER_ENABLED
#include "../../include/lights/LEDManager.h"

extern LEDManager ledManager;
#endif
Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

static uint8_t animFrame = 0;
static uint32_t lastAnim = 0;
int YvalAlignment = 30;  //  Create a Y Axis vertical alignment everything can be shifted up and down at once by adjusting this value.


ViewManager viewManager;

void ViewManager::begin(Adafruit_SH1107& disp) {
	_display = &disp;
	_display->begin(SCREEN_ADDRESS, true);
}

void ViewManager::draw() {
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
	_display->clearDisplay();
	_display->display();
}

void ViewManager::drawSplash() {
	_display->clearDisplay();
	int16_t bx, by;
	uint16_t w1, h1;
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 50), 10, voxel_100x65, 101, 65, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->setTextColor(SH110X_WHITE);
	_display->setTextSize(2);
	_display->getTextBounds(str_code_by, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 90);  //changed for SH1107
	printProgmemString(_display, str_code_by);
	_display->getTextBounds(str_3dmonkey, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 110);  //changed for SH1107
	printProgmemString(_display, str_3dmonkey);

	_display->display();
}

//====================================================================================================
// Placeholder for info screen (can be expanded with more details)
// Current operation state is isSystemStandby or isSystemOff
// Main standby screen with temperature, humidity, and system status
//====================================================================================================
void ViewManager::drawStandby() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->clearDisplay();
	_display->setTextSize(2);
	_display->setTextColor(SH110X_WHITE);
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

	// Show main status all centered at top
	_display->getTextBounds(msg, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 0);
	_display->print(msg);

	// Updated code to make the UI use the 128x128 screen.  Inclue the icons.h file for graphics.
	_display->setCursor(0, YvalAlignment);  // set curson to vertical alignment for TEMPERATURE Label

	_display->setTextSize(1);  // Set small text for the TEMPERATURE Label
	//_display->setCursor(0, YvalAlignment - 22);
	printProgmemString(_display, str_temp_label);                                                                  //  Set the TEMPERATURE Label up on Screen
	_display->setCursor(0, YvalAlignment + 9);                                                                     // set cursor to vertical alignment for Temperature Value
	_display->setTextSize(2);                                                                                      // Set the font bigger for the Temp Reading
	_display->print(sensorManager.getTemperature(), 2);                                                            // Get the Temp from the Sendor and setup on the screen
	printProgmemString(_display, str_space);                                                                       // Put a space between the temp numbers and The temp Unit
	_display->drawCircle(sensorManager.getTemperature() < 10.00f ? 55 : 67, YvalAlignment + 12, 3, SH110X_WHITE);  //  Draw the Degrees symbol
	printProgmemString(_display, getTemperatureUnit());                                                            //  Put the temp unit (C or F) on the screen
	//_display->setCursor(0, 35);
	_display->setTextSize(1);                          // Set the font smaller for the Humidity Label
	_display->setCursor(0, YvalAlignment + 26);        // set cursor to vertical alignment for Humidy Label
	printProgmemString(_display, str_humidity_label);  //Set the Humidity Label up on Screen
	_display->setCursor(0, YvalAlignment + 35);        // set cursor to vertical alignment for Humidy Value
	_display->setTextSize(2);                          // Set the font bigger for the Humidity Reading
	_display->print(sensorManager.getHumidity(), 2);   // Get the Humidity from the Sendor and setup on the screen
	printProgmemString(_display, str_percent);         //  Put the % sign on the screen

	_display->drawBitmap(96, (YvalAlignment - 3), thermometer_icon, 32, 32, SH110X_WHITE);  //  Draw a thermometer icon at the end of the temp printout
	_display->drawBitmap(96, (YvalAlignment + 24), humidity_icon, 32, 32, SH110X_WHITE);    //  Draw droplets icon at the end of the humidity printout

	// Show control mode or cooling status at bottom
	if (stateManager.isSystemOff() && fanState == FAN_COOLDOWN) {
		// Show COOLING at bottom when in OFF state and cooldown
		_display->setTextSize(1);
		_display->getTextBounds(str_cooling, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 20);
		printProgmemString(_display, str_cooling);
	}
	else if (!stateManager.isSystemOff()) {
		// Show control mode when not in OFF state
		_display->setTextSize(1);
		_display->setCursor(0, 20);
		printProgmemString(_display, str_mode_control);
		_display->setTextSize(1);
		switch (stateManager.getControlMode()) {
		case CONTROL_AUTO_TEMP: printProgmemString(_display, str_auto_temp); break;
		case CONTROL_AUTO_HUM: printProgmemString(_display, str_auto_hum); break;
		case CONTROL_USER_TEMP: printProgmemString(_display, str_user_temp); break;
		case CONTROL_USER_HUM: printProgmemString(_display, str_user_hum); break;
		default: _display->setTextColor(SH110X_WHITE); break;
		}
	}
	drawFanStatus();
#if !LED_MANAGER_ENABLED
	drawHeaterStatus();
#endif

#if LED_MANAGER_ENABLED
	drawLEDStatus();
#endif
	_display->display();
	// End of updated code /////////////////////////////////////////////////////////////////////////
}

void ViewManager::drawInfo() {
	extern StateManager stateManager;
	extern GyverPID heaterPID;
	_display->clearDisplay();
	uint32_t currentTime = millis();
	if (currentTime - lastAnim > systemIntervals.animation) {
		animFrame = !animFrame;
		lastAnim = currentTime;
	}
	float displayTargetTemp = stateManager.getTargetTemp();
	_display->fillRect(0, 0, 128, 13, SH110X_WHITE);
	_display->setTextColor(SH110X_BLACK);
	_display->setTextSize(1);
	_display->setCursor(4, 3);
	if (getThermalSecurityState() == THERMAL_PROTECTION && animFrame) {
		printProgmemString(_display, str_warning);
	}
	else {
		if (stateManager.getMode() == DRY_MODE_BY_HUM) {
			printProgmemString(_display, str_printing);
		}
		else {
			printProgmemString(_display, str_drying);
			uint32_t remainingSeconds = stateManager.getRemainingDryTime();
			uint16_t hours = remainingSeconds / SECONDS_PER_HOUR;
			uint16_t minutes = (remainingSeconds % SECONDS_PER_HOUR) / 60;
			char timeBuf[5];
			sprintf(timeBuf, "%02u:%02u", hours, minutes);
			_display->setCursor(70, 3);
			printProgmemString(_display, str_eta_label);
			_display->print(timeBuf);
		}
	}
	_display->setTextColor(SH110X_WHITE);
	_display->setFont(&FreeSans9pt7b);
	_display->setCursor(0, 30);
	_display->print(sensorManager.getTemperature(), 1);
	printProgmemString(_display, getTemperatureUnit());
	printProgmemString(_display, str_greater);
	_display->print(displayTargetTemp, 0);
	printProgmemString(_display, getTemperatureUnit());
	_display->setCursor(0, 48);
	_display->print(sensorManager.getHumidity(), 1);
	printProgmemString(_display, str_percent);
	if (stateManager.getMode() == DRY_MODE_BY_HUM) printProgmemString(_display, str_greater), _display->print(stateManager.getTargetHumidity()), printProgmemString(_display, str_percent_sign);
	_display->setCursor(0, 62);
	_display->setFont();
	_display->setTextSize(1);
	bool heaterOn = stateManager.isHeaterOn();
	_display->setCursor(3, 55);
	if (heaterOn && animFrame) _display->fillRect(0, 52, 60, 12, SH110X_WHITE);
	_display->setTextColor(heaterOn && animFrame ? SH110X_BLACK : SH110X_WHITE);
	if (heaterOn) {
		char heaterBuf[10];
		sprintf(heaterBuf, getProgmemString(str_heat_format), (int)(heaterPID.output * 100.0f / 255.0f));
		_display->print(heaterBuf);
	}
	else {
		printProgmemString(_display, str_heat_off);
	}
	ThermalSecurityState thermalState = getThermalSecurityState();
	FanState fanState = getFanState();
	bool fanActive = (thermalState == THERMAL_COOLDOWN) || (fanState == FAN_COOLDOWN) || isFanOn();
	_display->setCursor(70, 55);
	if (fanActive && animFrame) _display->fillRect(68, 52, 60, 12, SH110X_WHITE);
	_display->setTextColor(fanActive && animFrame ? SH110X_BLACK : SH110X_WHITE);
	printProgmemString(_display, (thermalState == THERMAL_COOLDOWN) ? str_balance : (fanState == FAN_COOLDOWN) ? str_cooling
		: isFanOn() ? str_cycling
		: str_fan_off);
	_display->setTextColor(SH110X_WHITE);
	drawFanStatus();
#if LED_MANAGER_ENABLED
	drawLEDStatus();
#endif
	_display->display();
}

//============================================================================================================
// This function initializes the layout for sub-screens (like settings screens) with a title and prepares
// the area for displaying the setting value.It also handles the text color based on whether the user
// is in edit mode or not, and includes a flashing effect for the editable value.
//============================================================================================================
void ViewManager::StartSubScreens(const char* title) {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->clearDisplay();
	_display->setTextSize(1);
	_display->setTextColor(SH110X_WHITE);
	_display->getTextBounds(title, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 0);
	printProgmemString(_display, title);
	_display->setTextSize(2);
	if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SH110X_WHITE);
	_display->setTextColor(stateManager.isEditing() ? SH110X_BLACK : SH110X_WHITE);
}

//============================================================================================================
// This function centers the provided label on the screen and is used for displaying the editable value in
// the configuration sub-screens. It calculates the text bounds of the label and sets the cursor position accordingly.
//============================================================================================================
void ViewManager::EditPartOfSubScreen(const char* label) {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->getTextBounds(label, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 23);
}

//============================================================================================================
// This function displays the "Change and Save" instruction at the bottom of the screen, prompting
// the user to save their changes after editing a setting. It is used across multiple configuration screens
// to maintain consistency in user instructions.
//============================================================================================================
void ViewManager::SaveEnterText() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->setTextColor(SH110X_WHITE);
	_display->setTextSize(1);
	_display->getTextBounds(str_change_save, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 50);
	printProgmemString(_display, str_change_save);
}

//============================================================================================================
// This function formats and displays the time based on the provided TimeVariable.
//============================================================================================================
void ViewManager::TimeConfiguration(uint32_t TimeVariable) {
	uint16_t hours = TimeVariable / 60;
	uint16_t minutes = TimeVariable % 60;
	char timeBuf[6];
	sprintf(timeBuf, "%02u:%02u", hours, minutes);
	EditPartOfSubScreen(timeBuf);
	_display->print(timeBuf);
}

//============================================================================================================
// This function draws the temperature configuration screen, allowing the user to set the target temperature.
// It includes a flashing effect on the temperature value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawTempConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_temp);
	// Convert numeric target temperature to a string for measurement and display
	String TargetTempStr = String(stateManager.getTargetTemp(), 0);  // 0 decimals
	const char* targetTempBuf = TargetTempStr.c_str();
	EditPartOfSubScreen(targetTempBuf);
	_display->print(stateManager.getTargetTemp());
	printProgmemString(_display, getTemperatureUnit());
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 14), 80, temp_icon_27x48, 27, 48, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}

//============================================================================================================
// This function draws the humidity configuration screen, allowing the user to set the target humidity.
// It includes a flashing effect on the humidity value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawHumConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_humidity);
	String targetHumStr = String(stateManager.getTargetHumidity(), 0);  // 0 decimals
	const char* targetHumBuf = targetHumStr.c_str();
	EditPartOfSubScreen(targetHumBuf);
	_display->print(stateManager.getTargetHumidity());
	printProgmemString(_display, str_percent);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, hum_icon_48x48, 48, 48, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}

//============================================================================================================
// This function draws the screen saver configuration screen, allowing the user to set the time until screen goes blank.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawScreenSaverSetting() {
	extern StateManager stateManager;
	StartSubScreens(str_screen_saver);
	char timeoutBuf[18];
	sprintf(timeoutBuf, "%u min", stateManager.getScreenTimeout());
	if (stateManager.getScreenTimeout() == 0)
		strcpy(timeoutBuf, getProgmemString(str_screen_saver_never));
	EditPartOfSubScreen(timeoutBuf);
	_display->print(timeoutBuf);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 25), 80, screen_saver_50x46, 50, 46, SH110X_WHITE);  //  Draw a timer icon at the end of the time printout
	_display->display();
}

//============================================================================================================
// This function draws the power outage memory screen, which informs the user about what happens after 
// a power outage and how the system retains settings and resumes operation. It includes relevant icons and instructions.
//============================================================================================================
void ViewManager::drawPowerOutageMemory() {
	extern StateManager stateManager;
	static bool isEditingLast = false;
	static int lastMode = -1;
	StartSubScreens(str_power_loss_memory_two);
	const char* powerOutLabel = stateManager.isPowerLossMemory() ? str_last_mode : str_power_off;
	EditPartOfSubScreen(powerOutLabel);
	printProgmemString(_display, powerOutLabel);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 25), 75, poweroutageicon_50x50, 50, 50, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}

//============================================================================================================
// This function draws the mode configuration screen, allowing the user to switch between humidity-based and time-based drying modes.
// It includes a flashing effect on the selected mode when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawModeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_operation_mode);
	const char* operatingModeLabel = stateManager.getMode() ? str_by_hum : str_dry;
	EditPartOfSubScreen(operatingModeLabel);
	printProgmemString(_display, operatingModeLabel);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, mode_icon_48x53, 48, 53, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}


//============================================================================================================
// This function draws the drying time configuration screen, allowing the user to set the duration for time-based drying mode.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawDryTimeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_mode_time);
	TimeConfiguration(stateManager.getDryDuration());
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, clock_icon_48x48, 48, 48, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}


//===========================================================================================================
// This function sets the time for when the system starts the Dry time again after it has been turned off,
// allowing the user to configure the delay before the system can be used again after being turned off.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//=============================================================================================================
void ViewManager::drawDryTimeStartConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_time_start);
	TimeConfiguration(stateManager.getDryStartTimer());
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, timer_icon_42x48, 42, 48, SH110X_WHITE);  //  Draw a filament icon at the end of the time printout
	_display->display();
}

//============================================================================================================
// This function displays the Fan status on the screen, showing the current fan speed as a percentage along with an icon.
// It includes a flashing effect when the fan is on or cooling.
//============================================================================================================
void ViewManager::drawFanStatus() {
	// Show Fan speed/status with label and icon, with flashing effect when fan is on or cooling
	_display->setTextSize(1);
	_display->setCursor(0, YvalAlignment + 52);
	_display->setTextColor(SH110X_WHITE);
	printProgmemString(_display, str_fan_format);
	_display->setCursor(0, YvalAlignment + 61);
	_display->setTextSize(2);
	_display->setTextColor(SH110X_WHITE);
	//	if (fanState != FAN_OFF && fanState != FAN_STANDBY_CYCLING_OFF && fanState != FAN_STANDBY_OFF) {
	if (getCurrentFanSpeed() > 0.0f) {
		_display->print((int)(getCurrentFanSpeed() * 100.0f / 255.0f));
		_display->setCursor((getCurrentFanSpeed() < 255.0f) ? ((2 * 12) + 5) : ((3 * 12) + 5), YvalAlignment + 61);
		printProgmemString(_display, str_percent);                                                                          //  Put the % sign on the screen
		_display->drawBitmap(96, (YvalAlignment + 58), animFrame ? fan_run1_32x31 : fan_run2_32x31, 32, 31, SH110X_WHITE);  //  Draw droplets icon at the end of the humidity printout
	}
	else {
		_display->setTextSize(2);
		_display->setCursor(0, YvalAlignment + 61);
		_display->setTextColor(SH110X_WHITE);
		printProgmemString(_display, str_fan_off);
		_display->drawBitmap(96, (YvalAlignment + 58), fan_still_32x31, 32, 31, SH110X_WHITE);  //  Draw droplets icon at the end of the humidity printout
	}
}

//============================================================================================================
// This function displays the Heater status on the screen, showing the current heater power as a percentage along with an icon.
// It includes a flashing effect when the heater is on.
//============================================================================================================
void ViewManager::drawHeaterStatus() {
	// Show heater status with label and icon, with flashing effect when heater is on
	bool heaterOn = stateManager.isHeaterOn();
	if (heaterOn && animFrame) _display->fillRect(0, 114, (4 * 12), 14, SH110X_WHITE);
	if (heaterOn) {
		_display->setTextSize(1);
		_display->setCursor(0, YvalAlignment + 76);
		_display->setTextColor(SH110X_WHITE);
		printProgmemString(_display, str_heater_format);
		_display->setCursor(0, YvalAlignment + 84);
		_display->setTextColor(heaterOn && animFrame ? SH110X_BLACK : SH110X_WHITE);
		_display->setTextSize(2);
		_display->print((int)(controlSystem.heaterPower * 100.0f / 255.0f));
		printProgmemString(_display, str_percent_sign);
		_display->drawBitmap(64, (YvalAlignment + 78), animFrame ? heater_icon1_25x20 : heater_icon2_25x20, 25, 20, SH110X_WHITE);  //  Draw a heater icon at the end of the temp printout, with flashing effect when heater is on
	}
	else {
		_display->setTextColor(SH110X_WHITE);
		_display->setCursor(0, YvalAlignment + 84);
		printProgmemString(_display, str_heat_off);
	}
}

//============================================================================================================
// This function displays the LED-strip status on the screen, showing the name of the currently playing LED
// pattern along with the brightness level. If the pattern name is too long to fit on the screen, 
// it implements a scrolling effect.
// It also includes a flashing effect when the LED pattern is active.
//============================================================================================================
#if LED_MANAGER_ENABLED
void ViewManager::drawLEDStatus() {

	// OLED Screen Led Pattern Label : Scrolling : TextSize = 2
	// Show Name of LED show that is playing - with scrolling if too long
	static int16_t scrollX = 0;
	static uint32_t lastScrollUpdate = 0;
	const uint32_t scrollInterval = 10;  // ms between scroll steps

	//Pause vars
	static uint32_t scrollPauseStart = 0;
	enum ScrollState {
		SCROLL_WAIT_START,
		SCROLLING,
		SCROLL_WAIT_END
	};
	static ScrollState scrollState = SCROLL_WAIT_START;
	const uint32_t scrollPauseDuration = 3000;  // 3 seconds pause

	int textSize = 1;              // _display->setTextSize()
	int charWidth = 6 * textSize;  // textSize = 1 or 2 : Automatically calculates the "close enough" width of each char
	String patternName = ledManager.getCurrentPatternName();
	int pixelWidth = patternName.length() * charWidth;
	int displayWidth = SCREEN_WIDTH;  // Width of the display in pixels

	_display->setTextSize(textSize);
	int yPos = 110;  // Adjust Y as needed for your layout

	if (pixelWidth <= displayWidth) {
		// Text fits, no scrolling needed
		scrollX = 0;  // reset scroll
		scrollState = SCROLL_WAIT_START;
		_display->setCursor(0, yPos);
		_display->print(patternName);
	}
	else {

		uint32_t now = millis();

		switch (scrollState) {
		case SCROLL_WAIT_START:
			if (scrollPauseStart == 0) {
				scrollPauseStart = now;  // Start the pause timer
			}

			// Stay in this state until pause duration is complete
			if (now - scrollPauseStart >= scrollPauseDuration) {
				scrollPauseStart = 0;  // Reset pause timer
				scrollState = SCROLLING;
			}

			// Show the text statically while waiting to scroll
			_display->setCursor(0, yPos);
			_display->print(patternName);
			break;


		case SCROLLING:
			if (now - lastScrollUpdate > scrollInterval) {
				scrollX--;
				lastScrollUpdate = now;

				if (scrollX < (-pixelWidth + displayWidth - 10)) {
					scrollState = SCROLL_WAIT_END;
					scrollPauseStart = now;
				}
			}
			_display->setCursor(scrollX, yPos);
			_display->print(patternName);
			break;

		case SCROLL_WAIT_END:
			_display->setCursor(scrollX, yPos);
			_display->print(patternName);

			if (now - scrollPauseStart > scrollPauseDuration) {
				scrollX = 0;
				scrollState = SCROLL_WAIT_START;
				scrollPauseStart = now;
			}
			break;
		}
	}
	_display->setCursor(0, yPos + 8);
	printProgmemString(_display, str_brightness_level);  //Set the Brightness Label up on Screen
	printProgmemString(_display, str_space);             // Put a space between the Brightness and the level number
	_display->print(ledManager.getBrightnessLevel());
}
#endif  // End of LED status function

//============================================================================================================
// This function displays an error message on the screen when there is an issue with the temperature sensor.
// It includes a blinking effect to draw attention to the error and provides clear instructions for the user
// to check the sensor connection.
//============================================================================================================
void ViewManager::TempSensorError() {
	static bool blink = false;
	_display->fillScreen(SH110X_BLACK);
	_display->setTextSize(1);
	_display->setTextColor(SH110X_WHITE);
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
		_display->fillRect(rectX2, rectY, rectW2, rectH, SH110X_BLACK);
		_display->setTextColor(SH110X_WHITE);
		// Print centered lines
		_display->setCursor(x1, firstBaselineY);
		_display->println(line1);
		_display->setCursor(x2, firstBaselineY + h1 + spacing);
		_display->println(line2);
	}
	else {
		// Erase only the text area
		_display->fillRect(rectX1, rectY, rectW1, rectH, SH110X_WHITE);
		_display->setTextColor(SH110X_BLACK);
		// Print centered lines
		_display->setCursor(x3, firstBaselineY);
		_display->println(line3);
		_display->setCursor(x2, firstBaselineY + h1 + spacing);
		_display->println(line2);
	}
	blink = !blink;
	_display->display();
}

#endif  // DISPLAY_TYPE_SH110X