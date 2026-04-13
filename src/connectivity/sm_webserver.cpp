/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
	 list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
	 list of conditions and the following disclaimer in the documentation and/or
	 other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
	 contributors may be used to endorse or promote products derived from
	 this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/sm_webserver.h"

// output buffer
char temp[35000];
// Copy PROGMEM into RAM (portable)
char fmtBuf[35000];

WebServer msserver(80);

const int led = 13;

SemiSmartWebServer sswServer;

void SemiSmartWebServer::smwebsetup(void) {
	if (MDNS.begin("esp32")) {
//		Serial.println("MDNS responder started");
	}

	msserver.on("/", std::bind(&SemiSmartWebServer::handleRoot, &sswServer));
	msserver.on("/sensors", std::bind(&SemiSmartWebServer::handleSensors, &sswServer));  // new sensors endpoint
	msserver.on("/setpower", std::bind(&SemiSmartWebServer::handlePower, &sswServer));
	msserver.on("/settargettemp", std::bind(&SemiSmartWebServer::handleTargetTemp, &sswServer));
	msserver.on("/settargethumidity", std::bind(&SemiSmartWebServer::handleTargetHumidity, &sswServer));
	msserver.on("/setmode", std::bind(&SemiSmartWebServer::handleSetMode, &sswServer));
	msserver.on("/setcontrolmode", std::bind(&SemiSmartWebServer::handleSetControlMode, &sswServer));
	msserver.on("/setdryduration", std::bind(&SemiSmartWebServer::handleSetDryDuration, &sswServer));
	msserver.on("/setidlestart", std::bind(&SemiSmartWebServer::handleSetIdleStart, &sswServer));
	msserver.on("/setscreensaver", std::bind(&SemiSmartWebServer::handleSetScreenSaver, &sswServer));
	msserver.on("/setpowerlossmemory", std::bind(&SemiSmartWebServer::handleSetPowerLossMemory, &sswServer));
	msserver.onNotFound(std::bind(&SemiSmartWebServer::handleNotFound, &sswServer));
	msserver.begin();
//	Serial.println("HTTP server started");
}

void SemiSmartWebServer::smwebloop(void) {
	msserver.handleClient();
	delay(2);
}

void SemiSmartWebServer::handleRoot() {
	digitalWrite(led, 1);


	// initial switch state mapping:
	// 0 = SYSTEM_OFF, 1 = SYSTEM_STANDBY (CYCLING), 2 = SYSTEM_ON (ACTIVE)
	int stateVal = 0;
	int cur = stateManager.getSystemState();
	if (cur == SYSTEM_OFF) stateVal = 0;
	else if (cur == SYSTEM_STANDBY) stateVal = 1;
	else if (cur == SYSTEM_ON) stateVal = 2;
	else stateVal = 0;

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
	size_t len = strlen(rootPageHtml);
	if (len >= sizeof(fmtBuf)) len = sizeof(fmtBuf) - 1;
	memcpy(fmtBuf, rootPageHtml, len);
	fmtBuf[len] = '\0';
#else
	strncpy_P(fmtBuf, rootPageHtml, sizeof(fmtBuf) - 1);
	fmtBuf[sizeof(fmtBuf) - 1] = '\0';
#endif

	// Replace the single "%d" placeholder manually to avoid interpreting other '%' sequences
	char* ph = strstr(fmtBuf, "%d");
	if (ph) {
		size_t prefixLen = ph - fmtBuf;
		const char* suffix = ph + 2;
		size_t suffixLen = strlen(suffix);
		if (prefixLen + 1 + suffixLen < sizeof(temp)) {
			memcpy(temp, fmtBuf, prefixLen);
			temp[prefixLen] = (char)('0' + (stateVal % 10));
			memcpy(temp + prefixLen + 1, suffix, suffixLen + 1);
		}
		else {
			strncpy(temp, fmtBuf, sizeof(temp) - 1);
			temp[sizeof(temp) - 1] = '\0';
		}
	}
	else {
		strncpy(temp, fmtBuf, sizeof(temp) - 1);
		temp[sizeof(temp) - 1] = '\0';
	}

	msserver.send(200, "text/html", temp);
	digitalWrite(led, 0);
}

void SemiSmartWebServer::handleSensors() {
	digitalWrite(led, 1);

	float t = sensorManager.getTemperature();
	float h = sensorManager.getHumidity();

	// currentState must be a SystemState variable
	// Example: SystemState currentState;
	uint8_t state = static_cast<uint8_t>(stateManager.getSystemState());
	uint8_t fanSpeed = (uint8_t)roundf(getCurrentFanSpeed() * 100.0f / 255.0f);
	uint8_t heatPower = (uint8_t)roundf(controlSystem.heaterPower * 100.0f / 255.0f);
	uint8_t oprMode = stateManager.getMode();
	uint8_t ctrlMode = stateManager.getControlMode();
	uint8_t autoPrintMode = printStatusSensor.isPrinting();
	uint32_t ETATotalSeconds = stateManager.getRemainingDryTime();
	uint32_t startDryTotalSec = ((stateManager.getDryStartTimer() * 60) - ((millis() - stateManager.getDryReStartTimer()) / 1000));
	if (startDryTotalSec > (DRY_START_MAX * 60))
		startDryTotalSec = 0;
	uint16_t ETAhours = ETATotalSeconds / SECONDS_PER_HOUR;
	uint16_t ETAminutes = (ETATotalSeconds % SECONDS_PER_HOUR) / 60;
	uint16_t ETAseconds = (ETATotalSeconds % 60);
	uint16_t startHours = startDryTotalSec / SECONDS_PER_HOUR;
	uint16_t startMinutes = (startDryTotalSec % SECONDS_PER_HOUR) / 60;
	uint16_t startSeconds = (startDryTotalSec % 60);
	bool manualTurnOff = stateManager.isManualTurnedOff();
	uint8_t targetTemp = stateManager.getTargetTemp();
	if (targetTemp < TEMP_MIN) targetTemp = TEMP_MIN;
	if (targetTemp > TEMP_MAX) targetTemp = TEMP_MAX;
	uint8_t th = stateManager.getTargetHumidity();
	if (th < HUM_MIN) th = HUM_MIN;
	if (th > HUM_MAX) th = HUM_MAX;
	uint16_t dryTimerDuration = stateManager.getDryDuration();
	uint16_t idleStartTimer = stateManager.getDryStartTimer();
	uint8_t screenSaverStartTime = stateManager.getScreenTimeout();
	bool powerOutageMemoryMode = stateManager.isPowerLossMemory();


	// JSON now includes: temp, hum, state, fanSpeed, heatPower
	char buf[1024];

	int n = snprintf(
		buf,
		sizeof(buf),
		"{\"temp\":%.2f,"
		"\"hum\":%.2f,"
		"\"state\":%u,"
		"\"fanSpeed\":%u,"
		"\"heatPower\":%u,"
		"\"oprMode\":%u,"
		"\"ctrlMode\":%u,"
		"\"autoPrintMode\":%u,"
		"\"startHours\":%u,"
		"\"startMinutes\":%u,"
		"\"startSeconds\":%u,"
		"\"manualTurnOff\":%u,"
		"\"dryTimerDuration\":%u,"
		"\"idleStartTimer\":%u,"
		"\"screenSaverStartTime\":%u,"
		"\"powerOutageMemoryMode\":%u,"
		"\"ETAhours\":%u,"
		"\"ETAminutes\":%u,"
		"\"ETAseconds\":%u,"
		"\"targetTemp\":%u,"
		"\"tempMin\":%u,"
		"\"tempMax\":%u,"
		"\"targetHumidity\":%u,"
		"\"humMin\":%u,"
		"\"humMax\":%u}",
		t, h, state, fanSpeed, heatPower, oprMode, ctrlMode,
		autoPrintMode, startHours, startMinutes, startSeconds, manualTurnOff,
		dryTimerDuration, idleStartTimer, screenSaverStartTime, powerOutageMemoryMode,
		ETAhours, ETAminutes, ETAseconds,
		targetTemp, TEMP_MIN, TEMP_MAX,
		th, HUM_MIN, HUM_MAX
	);

	if (n < 0) {
		msserver.send(500, "text/plain", "ERR");
	}
	else {
		msserver.send(200, "application/json", buf);
	}

	digitalWrite(led, 0);
}

void SemiSmartWebServer::handlePower() {
	digitalWrite(led, 1);

	String state = msserver.arg("state");  // expected "off","cycling","on" or numeric "0","1","2"

	// normalize parameter to integer value
	int newStateVal = -1;
	if (state == "0" || state.equalsIgnoreCase("off")) newStateVal = 0;
	else if (state == "1" || state.equalsIgnoreCase("cycling") || state.equalsIgnoreCase("standby")) newStateVal = 1;
	else if (state == "2" || state.equalsIgnoreCase("on") || state.equalsIgnoreCase("active")) newStateVal = 2;
	else {
		// fallback: if param not recognized, try to parse numeric
		newStateVal = state.toInt();
		if (newStateVal < 0 || newStateVal > 2) newStateVal = -1;
	}

	if (newStateVal != -1) {
		if (newStateVal == 0) {
			turnSystemOff();
		}
		else if (newStateVal == 1) {
			turnSystemStandby();
		}
		else {                                        // 2
			turnSystemOn();
		}

		// Always persist the new state
		saveSystemStateEEPROM(stateManager.getSystemState());
	}

	msserver.send(200, "text/plain", "OK");
	digitalWrite(led, 0);
}

void SemiSmartWebServer::handleTargetTemp() {
	digitalWrite(led, 1);

	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		digitalWrite(led, 0);
		return;
	}

	int v = msserver.arg("value").toInt();

	// Enforce system limits
	if (v < TEMP_MIN) v = TEMP_MIN;
	if (v > TEMP_MAX) v = TEMP_MAX;

	stateManager.setTargetTemp((uint8_t)v);

	msserver.send(200, "text/plain", "OK");
	digitalWrite(led, 0);
}

void SemiSmartWebServer::handleTargetHumidity() {
	digitalWrite(led, 1);

	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		digitalWrite(led, 0);
		return;
	}

	int v = msserver.arg("value").toInt();

	// Enforce system limits
	if (v < HUM_MIN) v = HUM_MIN;
	if (v > HUM_MAX) v = HUM_MAX;

	stateManager.setTargetHumidity((uint8_t)v);

	msserver.send(200, "text/plain", "OK");
	digitalWrite(led, 0);
}

void SemiSmartWebServer::handleSetMode() {
	digitalWrite(led, 1);

	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		digitalWrite(led, 0);
		return;
	}

	int v = msserver.arg("value").toInt();

	// Only allow 0 or 1
	if (v != 0 && v != 1) {
		msserver.send(400, "text/plain", "Invalid mode");
		digitalWrite(led, 0);
		return;
	}

	// Update backend state
	stateManager.setMode((OperationMode)v);

	// Persist if needed
	saveSystemStateEEPROM(stateManager.getSystemState());

	msserver.send(200, "text/plain", "OK");
	digitalWrite(led, 0);
}

void SemiSmartWebServer::handleSetControlMode() {
	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		return;
	}

	String mode = msserver.arg("value");

	if (mode == "CONTROL_USER_TEMP") stateManager.setControlMode(CONTROL_USER_TEMP);
	else if (mode == "CONTROL_USER_HUM") stateManager.setControlMode(CONTROL_USER_HUM);
	else if (mode == "CONTROL_AUTO_TEMP") stateManager.setControlMode(CONTROL_AUTO_TEMP);
	else if (mode == "CONTROL_AUTO_HUM") stateManager.setControlMode(CONTROL_AUTO_HUM);

	saveControlModeEEPROM(stateManager.getControlMode());

	msserver.send(200, "text/plain", "OK");
}

void SemiSmartWebServer::handleSetDryDuration() {
	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		return;
	}

	int duration = msserver.arg("value").toInt();
	if (duration < 10) duration = 10;
	if (duration > 720) duration = 720;

	stateManager.setDryDuration(duration);
	saveDryDurationEEPROM(duration);

	msserver.send(200, "text/plain", "OK");
}

void SemiSmartWebServer::handleSetIdleStart() {
	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		return;
	}

	int duration = msserver.arg("value").toInt();
	if (duration < 0) duration = 0;
	if (duration > 1440) duration = 1440;

	stateManager.setDryStartTimer(duration);
	saveDryStartTimeEEPROM(duration);

	msserver.send(200, "text/plain", "OK");
}

void SemiSmartWebServer::handleSetScreenSaver() {
	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		return;
	}

	int duration = msserver.arg("value").toInt();
	if (duration < 0) duration = 0;
	if (duration > 60) duration = 60;

	stateManager.setScreenTimeout(duration);
	saveScreenTimeoutEEPROM(duration);

	msserver.send(200, "text/plain", "OK");
}

void SemiSmartWebServer::handleSetPowerLossMemory() {
	if (!msserver.hasArg("value")) {
		msserver.send(400, "text/plain", "Missing value");
		return;
	}

	int mode = msserver.arg("value").toInt();
	if (mode < 0) mode = 0;
	if (mode > 1) mode = 1;

	stateManager.setPowerLossMemory(mode);
	savePowerLossMemoryEEPROM(mode);

	msserver.send(200, "text/plain", "OK");
}

void SemiSmartWebServer::handleNotFound() {
	digitalWrite(led, 1);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += msserver.uri();
	message += "\nMethod: ";
	message += (msserver.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += msserver.args();
	message += "\n";
	for (uint8_t i = 0; i < msserver.args(); i++) {
		message += " " + msserver.argName(i) + ": " + msserver.arg(i) + "\n";
	}
	msserver.send(404, "text/plain", message);
	digitalWrite(led, 0);
}


#endif