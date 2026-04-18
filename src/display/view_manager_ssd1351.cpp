#include "../../user_config.h"
#if DISPLAY_TYPE_SSD1351
#include "../../include/display/view_manager_ssd1351.h"

Adafruit_SSD1351 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);
//Adafruit_SSD1351 display(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);


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
int YvalAlignment = 30;  //  Create a Y Axis vertical alignment everything can be shifted up and down at once by adjusting this value.


ViewManager viewManager;

void ViewManager::begin(Adafruit_SSD1351& disp) {
	_display = &disp;
	_display->begin();
}

void ViewManager::draw() {
	static ViewID lastView = ViewID::RESET_SCREEN;
	if (_currentView != lastView) {
		_display->fillScreen(BLACK);
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
#if BLUETOOTH_WIFI_ENABLED
	case ViewID::BT_SCANNING: drawBTScanning(); break;
	case ViewID::BT_CONNECTED: drawBTConnected(); break;
	case ViewID::WIFI_SCANNING: drawWifiScanning(); break;
	case ViewID::WIFI_SELECT: drawWifiSelect(); break;
	case ViewID::WIFI_CONNECTED: drawWifiConnected(); break;
#endif
	default: drawInfo(); break;
	}
}

void ViewManager::ScreenSaverOn() {
	_display->fillScreen(BLACK);
}

void ViewManager::drawSplash() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->drawRGBBitmap((SCREEN_WIDTH / 2) - (124 / 2), 0, Voxel_logo_Color_124x78_inv, 124, 78);  //  Draw a Voxel Logo icon at the end of the temp printout
	_display->setTextColor(WHITE);
	_display->setTextSize(2);
	_display->getTextBounds(str_code_by, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 90);  //changed for SH1107
	printProgmemString(_display, str_code_by);
	_display->getTextBounds(str_3dmonkey, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 110);  //changed for SH1107
	printProgmemString(_display, str_3dmonkey);
}

//====================================================================================================
// Placeholder for info screen (can be expanded with more details)
// Current operation state is isSystemStandby or isSystemOff
// Main standby screen with temperature, humidity, and system status
//====================================================================================================
void ViewManager::drawStandby() {
	int16_t bx, by;
	uint16_t w1, h1;

	_display->setTextSize(2);
	_display->setTextColor(WHITE);
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
	//	_display->print(msg);	// Retrieve the current warning string and print it only once,
		// and again only if its content changes.
	if (!stringDisplayed || strcmp(msg, lastString) != 0) {
		// store the new warning text
		strncpy(lastString, msg, sizeof(lastString) - 1);
		lastString[sizeof(lastString) - 1] = '\0';
		_display->fillRect(0, 0, SCREEN_WIDTH, h1, BLACK);
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

	// Show control mode or cooling status at bottom
	if (stateManager.isSystemOff() && fanState == FAN_COOLDOWN) {
		// Show COOLING at bottom when in OFF state and cooldown
		_display->setTextSize(1);
		_display->getTextBounds(str_cooling, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), 20);
		_display->setTextColor(CYAN);
		if (isSystemOffLast != stateManager.isSystemOff()) {
			isSystemOffLast = stateManager.isSystemOff();
			_display->fillRect(0, 20, SCREEN_WIDTH, h1, BLACK);
			printProgmemString(_display, str_cooling);
		}
	}
	else if (stateManager.isSystemOff() && fanState != FAN_COOLDOWN) {
		_display->setTextSize(1);
		_display->getTextBounds(str_cooling, 0, 0, &bx, &by, &w1, &h1);
		_display->fillRect(0, 20, SCREEN_WIDTH, h1, BLACK);
	}
	else if (!stateManager.isSystemOff()) {
		// Show control mode when not in OFF state
		_display->setTextSize(1);
		_display->getTextBounds(str_auto_temp, 0, 0, &bx, &by, &w1, &h1);
		if (checkGetControlMode != stateManager.getControlMode()) {
			checkGetControlMode = stateManager.getControlMode();
			_display->setCursor(0, 20);
			_display->fillRect(0, 20, SCREEN_WIDTH, h1, BLACK);
			printProgmemString(_display, str_mode_control);
			_display->setTextSize(1);
			_display->setTextColor(checkGetControlMode == CONTROL_AUTO_TEMP || checkGetControlMode == CONTROL_AUTO_HUM ? PINK : YELLOW);
			switch (stateManager.getControlMode()) {
			case CONTROL_AUTO_TEMP: printProgmemString(_display, str_auto_temp); break;
			case CONTROL_AUTO_HUM: printProgmemString(_display, str_auto_hum); break;
			case CONTROL_USER_TEMP: printProgmemString(_display, str_user_temp); break;
			case CONTROL_USER_HUM: printProgmemString(_display, str_user_hum); break;
			default: _display->setTextColor(WHITE); break;
			}
			_display->setTextColor(WHITE);
		}
	}

	drawTemperature();
	drawHumidity();
	drawFanStatus();
#if !LED_MANAGER_ENABLED
	drawHeaterStatus();
#endif

#if LED_MANAGER_ENABLED
	drawLEDStatus();
#endif
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

	// End of updated code /////////////////////////////////////////////////////////////////////////
}

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
	float displayTargetTemp = stateManager.getTargetTemp();
	_display->fillRect(0, 0, 64, 13, WHITE);
	_display->setTextColor(BLACK);
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
			uint16_t seconds = (remainingSeconds % 60);
			char timeBuf[15];
			snprintf(timeBuf, sizeof(timeBuf), "%s %02u:%02u:%02u", getProgmemString(str_eta_label), hours, minutes, seconds);
			_display->setCursor(3, 68);
			_display->fillRect(0, 65, 77, 12, WHITE);
			_display->print(timeBuf);
		}
	}
	_display->setTextColor(WHITE);
	_display->setFont(&FreeSans9pt7b);
	_display->setCursor(0, YvalAlignment);
	_display->getTextBounds(str_greater, 0, 0, &bx, &by, &w1, &h1);
	if (sensorManager.getTemperature() != lastTemp) {
		lastTemp = sensorManager.getTemperature();
		_display->fillRect(0, YvalAlignment - 12, SCREEN_WIDTH, h1 + 6, BLACK);
		_display->print(sensorManager.getTemperature(), 1);
		printProgmemString(_display, getTemperatureUnit());
		printProgmemString(_display, str_greater);
		_display->print(displayTargetTemp, 0);
		printProgmemString(_display, getTemperatureUnit());
	}
	_display->setCursor(0, YvalAlignment + 18);

	if (sensorManager.getHumidity() != lastHum) {
		lastHum = sensorManager.getHumidity();
		_display->fillRect(0, YvalAlignment + 5, SCREEN_WIDTH, h1 + 6, BLACK);
		_display->print(sensorManager.getHumidity(), 1);
		printProgmemString(_display, str_percent);
		if (stateManager.getMode() == DRY_MODE_BY_HUM) printProgmemString(_display, str_greater), _display->print(stateManager.getTargetHumidity()), printProgmemString(_display, str_percent_sign);
	}
	_display->setCursor(0, 62);
	_display->setFont();
	_display->setTextSize(1);
	bool heaterOn = stateManager.isHeaterOn();
	_display->setCursor(3, 55);
	_display->fillRect(0, 52, 60, 12, (heaterOn ? animFrame ? YELLOW : RED : WHITE));
	_display->setTextColor(heaterOn ? animFrame ? RED : YELLOW : BLACK);
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
	if (fanActive && animFrame) _display->fillRect(68, 52, 60, 12, WHITE);
	_display->setTextColor(fanActive && animFrame ? BLACK : WHITE);
	printProgmemString(_display, (thermalState == THERMAL_COOLDOWN) ? str_balance : (fanState == FAN_COOLDOWN) ? str_cooling
		: isFanOn() ? str_cycling
		: str_fan_off);
	_display->setTextColor(WHITE);
	drawFanStatus();
#if LED_MANAGER_ENABLED
	drawLEDStatus();
#endif
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

}

//============================================================================================================
// This function initializes the layout for sub-screens (like settings screens) with a title and prepares
// the area for displaying the setting value.It also handles the text color based on whether the user
// is in edit mode or not, and includes a flashing effect for the editable value.
//============================================================================================================
void ViewManager::StartSubScreens(const char* title) {
	lastTemp = -1.0f; // reset last temp to force redraw of temp when entering temp config	
	lastHum = -1.0f; // reset last humidity to force redraw of humidity when entering temp config
	stringDisplayed = false;
	int16_t bx, by;
	uint16_t w1, h1;
	_display->setTextSize(1);
	_display->setTextColor(WHITE);
	_display->getTextBounds(title, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 0);
	printProgmemString(_display, title);
	_display->setTextSize(3);
}

//============================================================================================================
// This function displays the "Change and Save" instruction at the bottom of the screen, prompting
// the user to save their changes after editing a setting. It is used across multiple configuration screens
// to maintain consistency in user instructions.
//============================================================================================================
void ViewManager::SaveEnterText() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->setTextColor(WHITE);
	_display->setTextSize(1);
	_display->getTextBounds(str_change_save, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 50);
	printProgmemString(_display, str_change_save);
}

//============================================================================================================
// This function draws the temperature configuration screen, allowing the user to set the target temperature.
// It includes a flashing effect on the temperature value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawTempConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_temp);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static uint8_t lastTargetTemp = 0;

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
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getTargetTemp() != lastTargetTemp) {
			lastTargetTemp = stateManager.getTargetTemp();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	_display->print(tempBuf);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 14), 80, temp_icon_27x48, 27, 48, RED);  //  Draw a filament icon at the end of the time printout
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif
}

//============================================================================================================
// This function draws the humidity configuration screen, allowing the user to set the target humidity.
// It includes a flashing effect on the humidity value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawHumConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_set_humidity);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static uint8_t lastTargetHum = 0;
	//	_display->setTextSize(3);
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
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getTargetHumidity() != lastTargetHum) {
			lastTargetHum = stateManager.getTargetHumidity();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
 	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	_display->print(HumidBuf);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, hum_icon_48x48, 48, 48, BLUE);  //  Draw a filament icon at the end of the time printout
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

}

//============================================================================================================
// This function draws the screen saver configuration screen, allowing the user to set the time until screen goes blank.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and
// instructions.
//============================================================================================================
void ViewManager::drawScreenSaverSetting() {
	extern StateManager stateManager;
	StartSubScreens(str_screen_saver);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static int8_t lastTimeout = -1;
	_display->setTextSize(2);
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
			_display->fillRect(0, 18, SCREEN_WIDTH, h1 + 12, BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getScreenTimeout() != lastTimeout) {
			lastTimeout = stateManager.getScreenTimeout();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
 	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	printProgmemString(_display, timeoutBuf);
	SaveEnterText();
	_display->drawRGBBitmap((SCREEN_WIDTH / 2) - 25, 80, Screen_Saver_50x46_Color, 50, 46);  // Draw Power outage icon with color support
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif
}

//============================================================================================================
// This function draws the power outage memory screen, which informs the user about what happens after 
// a power outage and how the system retains settings and resumes operation. It includes relevant icons and instructions.
//============================================================================================================
void ViewManager::drawPowerOutageMemory() {
	extern StateManager stateManager;
	StartSubScreens(str_power_loss_memory_two);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static int8_t lastMode = -1;
	_display->setTextSize(2);
	_display->getTextBounds(stateManager.isPowerLossMemory() == false ? str_power_off : str_last_mode, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastMode = -1; // reset last mode to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.isPowerLossMemory() != lastMode) {
			lastMode = stateManager.isPowerLossMemory();
			_display->fillRect(0, 18, SCREEN_WIDTH, h1 + 12, BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.isPowerLossMemory() != lastMode) {
			lastMode = stateManager.isPowerLossMemory();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	printProgmemString(_display, stateManager.isPowerLossMemory() == false ? str_power_off : str_last_mode);
	SaveEnterText();
	_display->drawRGBBitmap((SCREEN_WIDTH / 2) - 25, 80, PowerOutageIcon_50x50_color, 50, 50);  // Draw Power outage icon with color support
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif
}

//============================================================================================================
// This function draws the mode configuration screen, allowing the user to switch between humidity-based and time-based drying modes.
// It includes a flashing effect on the selected mode when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawModeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_operation_mode);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static int8_t lastMode = -1;
	_display->setTextSize(2);
	_display->getTextBounds(stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry, 0, 0, &bx, &by, &w1, &h1);
	if (stateManager.isEditing() != isEditingLast) {
		isEditingLast = stateManager.isEditing();
		lastMode = -1; // reset last mode to force redraw when switching between edit and non-edit mode
	}
	if (stateManager.isEditing()) {
		if (stateManager.getMode() != lastMode) {
			lastMode = stateManager.getMode();
			_display->fillRect(0, 18, SCREEN_WIDTH, h1 + 12, BLACK);
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getMode() != lastMode) {
			lastMode = stateManager.getMode();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	printProgmemString(_display, stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, mode_icon_48x53, 48, 53, YELLOW);  //  Draw a filament icon at the end of the time printout
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

}


//============================================================================================================
// This function draws the drying time configuration screen, allowing the user to set the duration for time-based drying mode.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//============================================================================================================
void ViewManager::drawDryTimeConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_mode_time);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static uint32_t lastDuration = 0;
	//	_display->setTextSize(2);
	uint32_t duration = stateManager.getDryDuration();
	uint16_t hours = duration / MINUTES_PER_HOUR;
	uint16_t minutes = (duration % MINUTES_PER_HOUR);

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
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getDryDuration() != lastDuration) {
			lastDuration = stateManager.getDryDuration();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	_display->print(timeBuf);
	SaveEnterText();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, clock_icon_48x48, 48, 48, MAGENTA);  //  Draw a filament icon at the end of the time printout
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

}


//===========================================================================================================
// This function sets the time for when the system starts the Dry time again after it has been turned off,
// allowing the user to configure the delay before the system can be used again after being turned off.
// It includes a flashing effect on the time value when in edit mode and displays relevant icons and instructions.
//=============================================================================================================
void ViewManager::drawDryTimeStartConfig() {
	extern StateManager stateManager;
	StartSubScreens(str_dry_time_start);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool isEditingLast = false;
	static uint32_t lastStartTime = 0;
	//	_display->setTextSize(2);

	uint32_t startTimer = stateManager.getDryStartTimer();
	uint16_t hours = startTimer / MINUTES_PER_HOUR;
	uint16_t minutes = (startTimer % MINUTES_PER_HOUR);

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
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, WHITE);
		}
	}
	else {
		if (stateManager.getDryStartTimer() != lastStartTime) {
			lastStartTime = stateManager.getDryStartTimer();
			_display->fillRect((SCREEN_WIDTH / 2 - w1 / 2) - 7, 18, w1 + 10, h1 + 4, BLACK);
		}
	}
	_display->setTextColor(stateManager.isEditing() ? BLACK : WHITE);
	_display->getTextBounds(timerBuf, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 21);
	_display->print(timerBuf);
	_display->setTextColor(WHITE);
	_display->setTextSize(1);
	_display->getTextBounds(str_change_save, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), 50);
	printProgmemString(_display, str_change_save);
	_display->setFont();
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 24), 80, timer_icon_42x48, 42, 48, WHITE);  //  Draw a filament icon at the end of the time printout
#if BLUETOOTH_WIFI_ENABLED
	wifiStatusSymbol();
#endif

}

//============================================================================================================
// This function displays the Fan status on the screen, showing the current fan speed as a percentage along with an icon.
// It includes a flashing effect when the fan is on or cooling.
//============================================================================================================
void ViewManager::drawFanStatus() {
	// Show Fan speed/status with label and icon, with flashing effect when fan is on or cooling
	int16_t bx, by;
	uint16_t w1, h1;
	static bool FansOnState = false;
	_display->setTextSize(1);
	_display->setCursor(0, YvalAlignment + 52);
	_display->setTextColor(MAGENTA);
	printProgmemString(_display, str_fan_format);
	_display->setCursor(0, YvalAlignment + 61);
	_display->setTextSize(2);
	_display->setTextColor(WHITE);

	// Read fan speed and format into a string with 0 decimal places
	float currentfan = (getCurrentFanSpeed() * 100.0f / 255.0f);
	char FanBuffer[16];
	snprintf(FanBuffer, sizeof(FanBuffer), "%u%%", (unsigned int)currentfan);
	_display->getTextBounds(str_fan_off, 0, 0, &bx, &by, &w1, &h1);

	if (getCurrentFanSpeed() > 0.0f) {
		FansOnState = true;
		if (getCurrentFanSpeed() != lastFanSpeed) {
			lastFanSpeed = getCurrentFanSpeed();
			_display->fillRect(0, YvalAlignment + 61, w1, h1, BLACK);
		}
		_display->setTextColor(WHITE);
		_display->print(FanBuffer);  // Print formatted temperature string
		_display->fillRect(96, (YvalAlignment + 58), 32, 31, BLACK);
		_display->drawBitmap(96, (YvalAlignment + 58), animFrame ? fan_run1_32x31 : fan_run2_32x31, 32, 31, MAGENTA);  //  Draw droplets icon at the end of the humidity printout
	}
	else {
		if (getCurrentFanSpeed() != lastFanSpeed) {
			lastFanSpeed = getCurrentFanSpeed();
			_display->fillRect(0, YvalAlignment + 61, w1, h1, BLACK);
			_display->setTextColor(MAGENTA);
			printProgmemString(_display, str_fan_off);
		}
		if (FansOnState) {
			FansOnState = false;
			_display->fillRect(96, (YvalAlignment + 58), 32, 31, BLACK);
		}
		_display->drawBitmap(96, (YvalAlignment + 58), fan_still_32x31, 32, 31, MAGENTA);  //  Draw droplets icon at the end of the humidity printout
	}
}

//============================================================================================================
// This function displays the Heater status on the screen, showing the current heater power as a percentage along with an icon.
// It includes a flashing effect when the heater is on.
//============================================================================================================
void ViewManager::drawHeaterStatus() {
	// Show heater status with label and icon, with flashing effect when heater is on
	int16_t bx, by;
	uint16_t w1, h1;
	bool heaterOn = stateManager.isHeaterOn();
	if (heaterOn) {
		heaterOnState = true;
		_display->fillRect(0, 114, (4 * 12), 14, (animFrame ? YELLOW : RED));
		_display->setTextSize(1);
		_display->setTextColor(heaterOn && animFrame ? YELLOW : RED);
		_display->getTextBounds(str_heat_off, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor(0, YvalAlignment + 76);
		printProgmemString(_display, str_heater_format);
		_display->setCursor(0, YvalAlignment + 84);
		_display->setTextColor(animFrame ? RED : YELLOW);
		_display->setTextSize(2);
		_display->print((int)(controlSystem.heaterPower * 100.0f / 255.0f));
		printProgmemString(_display, str_percent_sign);
		_display->fillRect(60, (YvalAlignment + 78), 35, 20, BLACK);
		_display->drawBitmap(64, (YvalAlignment + 78), animFrame ? heater_icon1_25x20 : heater_icon2_25x20, 25, 20, animFrame ? YELLOW : RED);  //  Draw a heater icon at the end of the temp printout, with flashing effect when heater is on
	}
	else {
		if (heaterOnState) {
			heaterOnState = false;
			_display->fillRect(0, (YvalAlignment + 76), 50, 24, BLACK);
			_display->fillRect(60, (YvalAlignment + 78), 35, 20, BLACK);
		}
		_display->setTextColor(CYAN);
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
	int16_t bx, by;
	uint16_t w1, h1;
	static bool heightInitialized = false;
	int yPos = 110;  // Adjust Y as needed for your layout
	_display->setTextSize(1);
	_display->setCursor(0, yPos);
	_display->getTextBounds(ledManager.getCurrentPatternName(), 0, 0, &bx, &by, &w1, &h1);
	if (!stringDisplayed || strcmp(ledManager.getCurrentPatternName(), lastLEDPattern) != 0) {
		// store the new warning text
		strncpy(lastLEDPattern, ledManager.getCurrentPatternName(), sizeof(lastLEDPattern) - 1);
		lastLEDPattern[sizeof(lastLEDPattern) - 1] = '\0';
		_display->fillRect(0, yPos, (SCREEN_WIDTH * 0.75), h1 + 2, BLACK); // Clear area for scrolling text
		stringDisplayed = true;
	}
	_display->print(ledManager.getCurrentPatternName());
	if (!heightInitialized || lastBrightness != ledManager.getBrightnessLevel()) {
		heightInitialized = true;
		lastBrightness = ledManager.getBrightnessLevel();
		_display->fillRect(0, yPos + 8, (SCREEN_WIDTH * 0.75), h1, BLACK); // Clear area for scrolling text
	}
	_display->setCursor(0, yPos + 8);
	printProgmemString(_display, str_brightness_level);  //Set the Brightness Label up on Screen
	printProgmemString(_display, str_space);  // Put a space between the Brightness and the level number
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
	_display->fillScreen(BLACK);
	_display->setTextSize(1);
	_display->setTextColor(WHITE);
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
		_display->fillRect(rectX2, rectY, rectW2, rectH, BLACK);
		_display->setTextColor(WHITE);
		// Print centered lines
		_display->setCursor(x1, firstBaselineY);
		_display->println(line1);
		_display->setCursor(x2, firstBaselineY + h1 + spacing);
		_display->println(line2);
	}
	else {
		// Erase only the text area
		_display->fillRect(rectX1, rectY, rectW1, rectH, WHITE);
		_display->setTextColor(BLACK);
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
	_display->setCursor(0, YvalAlignment);  // set curson to vertical alignment for TEMPERATURE Label
	_display->setTextSize(1);  // Set small text for the TEMPERATURE Label
	_display->setTextColor(RED);
	printProgmemString(_display, str_temp_label);                                                                  //  Set the TEMPERATURE Label up on Screen
	_display->setCursor(0, YvalAlignment + 9);                                                                     // set cursor to vertical alignment for Temperature Value
	_display->setTextSize(2);                                                                                      // Set the font bigger for the Temp Reading
	_display->setTextColor(WHITE);
	// Read temperature and format into a string with 2 decimal places
	float currentTemp = sensorManager.getTemperature();
	char TempBuffer[16];
	snprintf(TempBuffer, sizeof(TempBuffer), "%.2f C", currentTemp);
	// Measure formatted text bounds
	_display->getTextBounds(TempBuffer, 0, 0, &bx, &by, &w1, &h1);
	if (currentTemp != lastTemp) {
		lastTemp = currentTemp;
		// Clear area large enough to accommodate temperature text
		_display->fillRect(0, YvalAlignment + 9, max((int)w1, (int)(SCREEN_WIDTH * 2 / 3)), h1, BLACK);
		_display->print(TempBuffer);  // Print formatted temperature string
	}
	_display->drawBitmap(96, (YvalAlignment - 3), thermometer_icon, 32, 32, RED);  //  Draw a thermometer icon at the end of the temp printout
}

//============================================================================================================
// This function displays the current humidity readings on the screen, along with corresponding icons.
// It includes a flashing effect when the humidity changes to draw attention to the updated reading.
//============================================================================================================
void ViewManager::drawHumidity() {
	int16_t bx, by;
	uint16_t w1, h1;
	_display->setCursor(0, YvalAlignment + 26);        // set cursor to vertical alignment for Humidy Label
	_display->setTextSize(1);                          // Set the font smaller for the Humidity Label
	_display->setTextColor(BLUE);
	printProgmemString(_display, str_humidity_label);  //Set the Humidity Label up on Screen
	_display->setCursor(0, YvalAlignment + 35);        // set cursor to vertical alignment for Humidy Value
	_display->setTextSize(2);                          // Set the font bigger for the Humidity Reading
	_display->setTextColor(WHITE);
	// Read humidity and format into a string with 2 decimal places
	float currentHum = sensorManager.getHumidity();
	char HumBuffer[16];
	snprintf(HumBuffer, sizeof(HumBuffer), "%.2f%%", currentHum);
	// Measure formatted text bounds
	_display->getTextBounds(HumBuffer, 0, 0, &bx, &by, &w1, &h1);

	if (sensorManager.getHumidity() != lastHum) {
		lastHum = sensorManager.getHumidity();
		_display->fillRect(0, YvalAlignment + 35, max((int)w1, (int)(SCREEN_WIDTH * 2 / 3)), h1, BLACK);
		_display->print(HumBuffer);  // Print formatted temperature string
	}
	_display->drawBitmap(96, (YvalAlignment + 24), humidity_icon, 32, 32, BLUE);    //  Draw droplets icon at the end of the humidity printout
}

#if BLUETOOTH_WIFI_ENABLED

//============================================================================================================
// This is the Bluetooth scanning screen, which shows an animation of a Bluetooth icon while the system is 
// scanning for Bluetooth devices. It will also show when the scanning is compete and connection is esablished.
// The animation is achieved by toggling the display of the Bluetooth icon every 500ms.
// ===========================================================================================================
void ViewManager::drawBTScanning() {
	StartSubScreens(str_BT_scanning);
	int16_t bx, by;
	uint16_t w1, h1;
	static bool BTiconFrame = false;
	static uint32_t lastBTAnim = 0;
	static int instructionDisplayed = -1;
	if (instructionDisplayed != BTiconFrame) {
		instructionDisplayed = BTiconFrame;
		_display->setTextSize(1);
		_display->fillRect(5, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 10, 55, (BTiconFrame == 0 ? BLACK : RED)); // Clear area for instructions
		_display->setTextColor(WHITE);
		_display->getTextBounds(str_exit_config2, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), SCREEN_HEIGHT - 1 - (h1 + 5));
		_display->print(getProgmemString(str_exit_config2));
		_display->getTextBounds(str_exit_config1, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor((SCREEN_WIDTH / 2) - (w1 / 2), SCREEN_HEIGHT - 1 - (2 * (h1 + 5)));
		_display->print(getProgmemString(str_exit_config1));
	}

	if (millis() - lastBTAnim > 500) { // Change frame every 500ms
		lastBTAnim = millis();
		BTiconFrame = !BTiconFrame;
		_display->fillRect((SCREEN_WIDTH / 2) - 22, (SCREEN_HEIGHT / 2) - 35, 44, 50, BLACK); // Clear the Bluetooth icon to create a blinking effect
		if (!BTiconFrame) {
			_display->drawBitmap((SCREEN_WIDTH / 2) - 22, (SCREEN_HEIGHT / 2) - 35, bt_icon_44x50, 44, 50, BLUE);  //  Draw a Bluetooth icon below the text
		} 
	}
}

/*
* This screen is shown when the Bluetooth connection is successful,
* and shows the device name if available.
*/
void ViewManager::drawBTConnected() {
	stringDisplayed = false;
	StartSubScreens(str_bluetooth_connected);
	_display->drawBitmap((SCREEN_WIDTH / 2) - 22, (SCREEN_HEIGHT / 2) - 35, bt_icon_44x50, 44, 50, BLUE);  //  Draw a Bluetooth icon below the text
}


//============================================================================================================
// This function display that the Bluetooth connection is successful, and shows the device name if available.
// The Wifi scanning is then started and is shown
//============================================================================================================
void ViewManager::drawWifiScanning() {
	StartSubScreens(str_wifi);
	int16_t bx, by;
	uint16_t w1, h1;
	uint16_t HeightOnScreen = 11;
	const uint8_t PixelsBetweenRows = 3;
	static bool NoNetworsFoundDisplayed = false;
	if (wifiExztra.isNumberofNetworksFound() == 0 && !NoNetworsFoundDisplayed) {
		_display->setTextSize(1);
		_display->getTextBounds(str_wifi_no_networks_1, 0, 0, &bx, &by, &w1, &h1);
		_display->setCursor(((SCREEN_WIDTH / 2) - (w1 / 2)), (SCREEN_HEIGHT / 2) - (h1 / 2));
		printProgmemString(_display, str_wifi_no_networks_1);
		_display->drawBitmap(((SCREEN_WIDTH / 2) - 25), SCREEN_HEIGHT - 38, nowifi_icon_50x38, 50, 38, YELLOW);  //  Draw a WiFi icon below the text
		NoNetworsFoundDisplayed = true;
	}
	else if (wifiExztra.isNumberofNetworksFound() > 0) {
		if (NoNetworsFoundDisplayed) {
			_display->setTextSize(1);
			_display->getTextBounds(str_wifi_no_networks_1, 0, 0, &bx, &by, &w1, &h1);
			_display->fillRect(0, h1, SCREEN_WIDTH, (SCREEN_HEIGHT - h1), BLACK);
			NoNetworsFoundDisplayed = false;
		}
	_display->setTextSize(1);
		_display->setCursor(1, HeightOnScreen);
		printProgmemString(_display, str_wifi_list_divider);
		printProgmemString(_display, str_wifi_list_networks_Nr);
		printProgmemString(_display, str_wifi_list_divider);
		printProgmemString(_display, str_wifi_list_networks_SSID);
		_display->getTextBounds(str_wifi_list_networks_SSID, 0, 0, &bx, &by, &w1, &h1);
		HeightOnScreen += h1 + PixelsBetweenRows;
		for (int i = 0; i < wifiExztra.isNumberofNetworksFound() && i < MAX_WIFI_NETWORKS; i++) {
			_display->setCursor(1, HeightOnScreen + (i * (h1 + PixelsBetweenRows)));
			printProgmemString(_display, str_wifi_list_divider);
			_display->printf("%2d", i + 1);
			printProgmemString(_display, str_wifi_list_divider);
			static char ssidBuf[33];
			ssidBuf[0] = '\0';
			strncpy(ssidBuf, wifiExztra.getyourSSID(i).c_str(), sizeof(ssidBuf) - 1);
			ssidBuf[sizeof(ssidBuf) - 1] = '\0';
			_display->printf("%-17.17s", ssidBuf);
			_display->setTextSize(1);
			_display->getTextBounds(str_bluetooth_instruction3, 0, 0, &bx, &by, &w1, &h1);
			_display->setTextColor(WHITE);
			_display->setCursor(0, SCREEN_HEIGHT - 20 - (1 * h1));
			_display->print(getProgmemString(str_bluetooth_instruction3));
			_display->setCursor(0, SCREEN_HEIGHT - 20 - (2 * h1));
			_display->print(getProgmemString(str_bluetooth_instruction2));
			_display->setCursor(0, SCREEN_HEIGHT - 20 - (3 * h1));
			_display->print(getProgmemString(str_bluetooth_instruction1));
			_display->drawBitmap(((SCREEN_WIDTH / 2) - 25), SCREEN_HEIGHT - 20, wifi_icon_50x36, 50, 36, YELLOW);  //  Draw a WiFi icon below the text
		}
	}
}


//============================================================================================================
// This function displays the WiFi network selection screen, allowing the user to choose from the list of
// available WiFi networks. It shows the list of networks with their corresponding numbers for selection and includes
// an icon to indicate WiFi settings.
//============================================================================================================
void ViewManager::drawWifiSelect() {
	StartSubScreens(str_wifi);
	int16_t bx, by;
	uint16_t w1, h1;
	uint16_t HeightOnScreen = 11;
	const uint8_t PixelsBetweenRows = 3;

	_display->drawBitmap((SCREEN_WIDTH / 2) - 25, HeightOnScreen, wifi_icon_50x36, 50, 36, YELLOW);  //  Draw a WiFi icon below the text
	_display->setTextSize(1);
	HeightOnScreen += 36;
	_display->getTextBounds(str_wifi_list_networks_SSID, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(10, HeightOnScreen);
	_display->printf("%-32s", str_wifi_list_networks_SSID);
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setTextSize(2);
	_display->setCursor(10, HeightOnScreen);
	String selectedSSID = wifiExztra.getTheSSID();
	if (!selectedSSID.isEmpty()) {
		_display->printf("%-32s", selectedSSID.c_str());
	}
	else {
		_display->print(getProgmemString(str_wifi_no_networks_1));
	}
	HeightOnScreen += (2 * h1) + PixelsBetweenRows;
	_display->setTextSize(1);
	_display->getTextBounds(str_bluetooth_instruction1, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(5, HeightOnScreen);
	_display->print(getProgmemString(str_bluetooth_instruction1));
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(5, HeightOnScreen);
	_display->print(getProgmemString(str_bluetooth_instruction4));
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setCursor(5, HeightOnScreen);
	_display->print(getProgmemString(str_bluetooth_instruction5));

}

//============================================================================================================
// This function displays the WiFi connection successful screen, showing the connected network's SSID and
// an icon to indicate successful connection. It is shown after the user selects a WiFi network and the connection is established.
//============================================================================================================
void ViewManager::drawWifiConnected() {
	StartSubScreens(str_wifi_connected_3);
	int16_t bx, by;
	uint16_t w1, h1;
	uint16_t HeightOnScreen = 11;
	const uint8_t PixelsBetweenRows = 2;
	static uint8_t wifiStatusTemp = -1;
	static uint8_t currentStatus = -1;
	static uint32_t lastStatusChangeTime = 0;
	static bool blinker = false;


	_display->setTextSize(1);
	_display->getTextBounds(str_wifi_list_networks_SSID, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(10, HeightOnScreen);
	_display->printf("%-32s", str_wifi_list_networks_SSID);
	HeightOnScreen += h1 + PixelsBetweenRows;
	_display->setTextSize(2);
	_display->setCursor(10, HeightOnScreen);
	String selectedSSID = wifiExztra.getTheSSID();
	// Ensure the SSID is not too long for the display
	// We will print a maximum of 32 characters for the SSID, and if it's longer, it will be truncated with "..." at the end.
	if (selectedSSID.length() <= 15) {
		_display->getTextBounds(selectedSSID.c_str(), 0, 0, &bx, &by, &w1, &h1);
		if (selectedSSID.length() > 12) {
			selectedSSID = selectedSSID.substring(0, 9) + "...";
		}
		_display->printf("%-12.12s", selectedSSID.c_str());
	}
	else if (selectedSSID.length() > 15) {
		_display->setTextSize(1);
		_display->getTextBounds(selectedSSID.c_str(), 0, 0, &bx, &by, &w1, &h1);
		if (selectedSSID.length() > 25) {
			selectedSSID = selectedSSID.substring(0, 22) + "...";
		}
		_display->printf("%-25.25s", selectedSSID.c_str());
	}
	HeightOnScreen += h1 + (2 * PixelsBetweenRows);

	_display->setTextSize(1);
	_display->getTextBounds(str_wifi_status, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor(0, HeightOnScreen);
	_display->print(getProgmemString(str_wifi_status));

	currentStatus = wifiExztra.getWiFiStatus();
	HeightOnScreen += h1 + PixelsBetweenRows;

	if (wifiStatusTemp != currentStatus) {
		wifiStatusTemp = currentStatus;
		_display->getTextBounds(str_wifi_status, 0, 0, &bx, &by, &w1, &h1);
		_display->fillRect(0, HeightOnScreen, SCREEN_WIDTH, h1, BLACK);
		_display->setCursor(0, HeightOnScreen);
		switch (currentStatus) {
		case WL_NO_SSID_AVAIL: _display->print(getProgmemString(str_wifi_no_networks_1)); break;
		case WL_SCAN_COMPLETED: _display->print(getProgmemString(str_wifi_scan_completed_2)); break;
		case WL_CONNECTED: _display->print(getProgmemString(str_wifi_connected_3)); break;
		case WL_CONNECT_FAILED: _display->print(getProgmemString(str_connection_failed_4)); break;
		case WL_CONNECTION_LOST: _display->print(getProgmemString(str_connection_lost_5)); break;
		case WL_DISCONNECTED: _display->print(getProgmemString(str_wifi_disconnected_6)); break;
		}
	}
	HeightOnScreen += h1 + (2 * PixelsBetweenRows);

	_display->setTextSize(1);
	_display->setCursor(0, HeightOnScreen);
	String ipAddress = wifiExztra.getIPAddress();
	_display->getTextBounds(ipAddress, 0, 0, &bx, &by, &w1, &h1);
	if (!ipAddress.isEmpty()) {
		_display->printf("IP:%s", ipAddress.c_str());
	}
	HeightOnScreen += h1 + PixelsBetweenRows;

	_display->setTextSize(1);
	_display->setCursor(0, HeightOnScreen);
	String macAddress = wifiExztra.getMACAddress();
	_display->getTextBounds(macAddress, 0, 0, &bx, &by, &w1, &h1);
	if (!macAddress.isEmpty()) {
		_display->printf("MAC:%s", macAddress.c_str());
	}
	HeightOnScreen += h1 + PixelsBetweenRows;

	_display->setTextSize(1);
	_display->getTextBounds(str_wifi_continue, 0, 0, &bx, &by, &w1, &h1);
	_display->setCursor((SCREEN_WIDTH - w1) / 2, SCREEN_HEIGHT - 23 - (h1 + PixelsBetweenRows));
	if (millis() - lastStatusChangeTime > 500) { // Blink every 500ms
		lastStatusChangeTime = millis();
		blinker = !blinker;
	}
	_display->fillRect(0, SCREEN_HEIGHT - 25 - (h1 + PixelsBetweenRows), SCREEN_WIDTH, h1 + (PixelsBetweenRows * 2), blinker ? BLACK : RED); // Clear previous text
	_display->print(getProgmemString(str_wifi_continue));
	_display->drawBitmap(((SCREEN_WIDTH / 2) - 25), SCREEN_HEIGHT - 20, wifi_icon_50x36, 50, 36, YELLOW);  //  Draw a WiFi icon below the text
}


/*
* Display the Wifi status symbol on the lower right part of the screen
*/
void ViewManager::wifiStatusSymbol() {
	static int wifiStatusBuff = -1;
	if (wifiStatusBuff != wifiExztra.getWiFiStatus()) {
		wifiStatusBuff = wifiExztra.getWiFiStatus();
		_display->fillRect((SCREEN_WIDTH - 31), (SCREEN_HEIGHT - 23), 31, 23, BLACK);
	}
	if (wifiExztra.getWiFiStatus() == WL_CONNECTED)
		_display->drawBitmap((SCREEN_WIDTH - 20), 0, wifi_icon_20x14, 20, 14, YELLOW);  //  Draw a thermometer icon at the end of the temp printout
	else
		_display->drawBitmap((SCREEN_WIDTH - 20), 0, nowifi_icon_20x15, 20, 15, RED);  //  Draw a thermometer icon at the end of the temp printout
}


#endif // BLUETOOTH_WIFI_ENABLED

#endif  // DISPLAY_TYPE_SSD1351