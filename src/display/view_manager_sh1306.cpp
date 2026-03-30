#include "../../include/safety/thermal_security.h"
#if DISPLAY_TYPE_SSD1306
#include "../../include/display/view_manager_sh1306.h"
#include "../../include/control/control.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static uint8_t animFrame = 0;
static uint32_t lastAnim = 0;


ViewManager viewManager;

void ViewManager::begin(Adafruit_SSD1306& disp) {
	_display = &disp;
	_display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
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

	_display->setTextColor(SSD1306_WHITE);
	_display->setTextSize(2);
	_display->setCursor(24, 15);
	printProgmemString(_display, str_voxel3d);

	_display->setTextSize(1);
	_display->setCursor(15, 44);
	printProgmemString(_display, str_code_by);

	_display->display();
}

//====================================================================================================
// Placeholder for info screen (can be expanded with more details)
// Current operation state is isSystemStandby or isSystemOff
// Main standby screen with temperature, humidity, and system status
//====================================================================================================
void ViewManager::drawStandby() {
	_display->clearDisplay();
	_display->setTextSize(2);
	_display->setTextColor(SSD1306_WHITE);
	static uint8_t animFrame = 0;
	static uint32_t lastAnim = 0;
	uint32_t currentTime = millis();
	if (currentTime - lastAnim > systemIntervals.animation) {
		animFrame = !animFrame;
		lastAnim = currentTime;
	}
	bool heaterOn = stateManager.isHeaterOn();
	const char* msg;
	FanState fanState = getFanState();

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
	_display->setCursor(stateManager.isSystemOff() ? 5 : 18, 4);
	_display->print(msg);

	_display->setTextSize(1);
	_display->setCursor(0, 22);
	printProgmemString(_display, str_temp_label);        //  Set the TEMPERATURE Label up on Screen
	_display->print(sensorManager.getTemperature(), 1);  // Get the Temp from the Sendor and setup on the screen

	// Draw a small superimposed circle after the temperature value.
	// Use current cursor position to place the circle so it appears immediately after the number.
	{
		int16_t curX = _display->getCursorX();
		int16_t curY = _display->getCursorY();
		// Adjust offsets so circle sits near the top-right of the printed digits (tweak if needed)
		int16_t circleX = curX + 3;
		int16_t circleY = curY;
		_display->drawCircle(circleX, circleY, 1, SSD1306_WHITE);
	}

	printProgmemString(_display, str_space);  // Put a space between the temp numbers and The temp Unit
	// Degrees symbol / temperature unit
	printProgmemString(_display, getTemperatureUnit());  //  Put the temp unit (C or F) on the screen
	_display->setCursor(0, 32);
	printProgmemString(_display, str_humidity_label);  //Set the Humidity Label up on Screen
	_display->print(sensorManager.getHumidity(), 2);   // Get the Humidity from the Sendor and setup on the screen
	printProgmemString(_display, str_percent);         //  Put the % sign on the screen
	_display->setCursor(0, 43);
	if (heaterOn) {
		if (animFrame)
			_display->setTextColor(BLACK, WHITE);
		else
			_display->setTextColor(WHITE, BLACK);
		char heaterBuf[10];
		sprintf(heaterBuf, getProgmemString(str_heat_format), (int)(controlSystem.heaterPower * 100.0f / 255.0f));
		_display->print(heaterBuf);
	}
	else {
		_display->setTextColor(SSD1306_WHITE);
		printProgmemString(_display, str_heat_off);
	}

	// Show control mode or cooling status at bottom
	_display->setTextColor(SSD1306_WHITE);
	if (stateManager.isSystemOff() && fanState == FAN_COOLDOWN) {
		// Show COOLING at bottom when in OFF state and cooldown
		_display->setTextSize(1);
		_display->setCursor(0, 55);
		printProgmemString(_display, str_cooling);
	}
	else if (!stateManager.isSystemOff()) {
		// Show control mode when not in OFF state
		_display->setTextSize(1);
		_display->setCursor(0, 54);
		printProgmemString(_display, str_mode_control);
		switch (stateManager.getControlMode()) {
		case CONTROL_AUTO_TEMP: printProgmemString(_display, str_auto_temp); break;
		case CONTROL_AUTO_HUM: printProgmemString(_display, str_auto_hum); break;
		case CONTROL_USER_TEMP: printProgmemString(_display, str_user_temp); break;
		case CONTROL_USER_HUM: printProgmemString(_display, str_user_hum); break;
		default: _display->setTextColor(SSD1306_WHITE); break;
		}
	}
	_display->display();
	// End of updated code /////////////////////////////////////////////////////////////////////////
}

void ViewManager::drawInfo() {
	_display->clearDisplay();
	uint32_t currentTime = millis();
	if (currentTime - lastAnim > systemIntervals.animation) {
		animFrame = !animFrame;
		lastAnim = currentTime;
	}
	float displayTargetTemp = stateManager.getTargetTemp();
	_display->fillRect(0, 0, 128, 13, SSD1306_WHITE);
	_display->setTextColor(SSD1306_BLACK);
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
	_display->setTextColor(SSD1306_WHITE);
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
	if (heaterOn) {
		if (animFrame)
			_display->setTextColor(BLACK, WHITE);
		else
			_display->setTextColor(WHITE, BLACK);
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
	//	if (fanActive && animFrame) _display->fillRect(68, 52, 60, 12, SSD1306_WHITE);
	if (fanActive && animFrame)
		_display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
	else
		_display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
	printProgmemString(_display, (thermalState == THERMAL_COOLDOWN) ? str_balance : (fanState == FAN_COOLDOWN) ? str_cooling
		: isFanOn() ? str_cycling
		: str_fan_off);
	_display->setTextColor(SSD1306_WHITE);
	_display->display();
}

//============================================================================================================
// This function initializes the layout for sub-screens (like settings screens) with a title and prepares
// the area for displaying the setting value.It also handles the text color based on whether the user
// is in edit mode or not, and includes a flashing effect for the editable value.
//============================================================================================================
void ViewManager::StartSubScreens(const char* title, uint8_t xpos) {
	_display->clearDisplay();
	_display->setTextSize(1);
	_display->setTextColor(SSD1306_WHITE);
	_display->setCursor(0, 0);
	printProgmemString(_display, title);
	_display->setTextSize(2);
	if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SSD1306_WHITE);
	_display->setTextColor(stateManager.isEditing() ? SSD1306_BLACK : SSD1306_WHITE);
	_display->setCursor(xpos, 23);
}

//============================================================================================================
// This function displays the "Change and Save" instruction at the bottom of the screen, prompting
// the user to save their changes after editing a setting. It is used across multiple configuration screens
// to maintain consistency in user instructions.
//============================================================================================================
void ViewManager::SaveEnterText() {
	_display->setTextColor(SSD1306_WHITE);
	_display->setTextSize(1);
	_display->setCursor(0, 50);
	printProgmemString(_display, str_change_save);
	_display->display();
}

//============================================================================================================
// This function formats and displays the time based on the provided TimeVariable.
//============================================================================================================
void ViewManager::TimeConfiguration(uint32_t TimeVariable) {
	uint16_t hours = TimeVariable / 60;
	uint16_t minutes = TimeVariable % 60;
	char timeBuf[6];
	sprintf(timeBuf, "%02u:%02u", hours, minutes);
	_display->print(timeBuf);
}

//============================================================================================================
// This function draws the temperature configuration screen, allowing the user to set the target temperature.
// It includes a flashing effect on the temperature value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawTempConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_temp, 40);
	_display->print(stateManager.getTargetTemp());
	printProgmemString(_display, getTemperatureUnit());
	SaveEnterText();
}

//============================================================================================================
// This function draws the humidity configuration screen, allowing the user to set the target humidity.
// It includes a flashing effect on the humidity value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawHumConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_humidity, 40);
	_display->print(stateManager.getTargetHumidity());
	printProgmemString(_display, str_percent);
	SaveEnterText();
}

//============================================================================================================
// This function draws the screen saver configuration screen, allowing the user to set the time until screen goes blank.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawScreenSaverSetting() {
	extern StateManager stateManager;
	StartSubScreens(str_screen_saver, 10);
	char timeoutBuf[6];
	sprintf(timeoutBuf, "%u min", stateManager.getScreenTimeout());
	if (stateManager.getScreenTimeout() == 0)
		strcpy(timeoutBuf, getProgmemString(str_screen_saver_never));
	_display->print(timeoutBuf);
	SaveEnterText();
}


//============================================================================================================
// This function draws the power outage memory screen, which informs the user about what happens after
// a power outage and how the system retains settings and resumes operation. It includes relevant icons and instructions.
//============================================================================================================
void ViewManager::drawPowerOutageMemory() {
	extern StateManager stateManager;
	StartSubScreens(str_power_loss_memory_two, 10);
	printProgmemString(_display, stateManager.isPowerLossMemory() == false ? str_power_off : str_last_mode);
	SaveEnterText();
}

//============================================================================================================
// This function draws the mode configuration screen, allowing the user to switch between humidity-based and time-based drying modes.
// It includes a flashing effect on the selected mode when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawModeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_operation_mode, 10);
	printProgmemString(_display, stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry);
	SaveEnterText();
}

//============================================================================================================
// This function draws the drying time configuration screen, allowing the user to set the duration for time-based drying mode.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawDryTimeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_mode_time, 30);
	TimeConfiguration(stateManager.getDryDuration());
	SaveEnterText();
}

//===========================================================================================================
// This function sets the time for when the system starts the Dry time again after it has been turned off,
// allowing the user to configure the delay before the system can be used again after being turned off.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//=============================================================================================================

void ViewManager::drawDryTimeStartConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_time_start, 30);
	TimeConfiguration(stateManager.getDryStartTimer());
	SaveEnterText();
}

//============================================================================================================
// This function displays an error message on the screen when there is an issue with the temperature sensor
// It prompts the user to check the wiring and ensures that the error message is clear and visible.
//============================================================================================================
void ViewManager::TempSensorError() {
	_display->clearDisplay();
	_display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
	const char* line1 = "No Sensor";
	const char* line2 = "Check wiring";
	_display->setCursor(12, 10);
	_display->setTextSize(2);
	_display->print(line1);
	_display->setTextSize(1);
	_display->setCursor(30, 30);
	_display->println(line2);
	_display->display();
}

#endif  // DISPLAY_TYPE_SSD1306