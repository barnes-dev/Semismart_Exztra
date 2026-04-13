#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>
#include "system_config.h"

// System states
enum SystemState : uint8_t {
	SYSTEM_OFF,      // Completely off, no cycling
	SYSTEM_STANDBY,  // With air cycling
	SYSTEM_ON        // Normal operation
};

enum class ViewID : uint8_t {
	SPLASH = 0,     // New splash screen with logo
	STANDBY = 1,
	INFO = 2,
	TEMP = 3,
	HUM = 4,
	MODE = 5,
	POWEROUT = 6,
	DRY_TIME = 7,
	DRY_START = 8,
	SENSOR_ERROR = 9,
	SCREEN_SAVER = 10,
	SCREEN_OFF = 11,
#if BLUETOOTH_WIFI_ENABLED
	BT_SCANNING = 12,
	BT_CONNECTED = 13,
	WIFI_SCANNING = 14,
	WIFI_SELECT = 15,
	WIFI_CONNECTED = 16,
#endif
	RESET_SCREEN = 17
};

class StateManager {
public:
	// Initialization
	void begin();

	// System state
	bool isSystemOn() const { return _systemState == SYSTEM_ON; }
	void setSystemOn(bool on) {
		if (on) {
			_systemState = SYSTEM_ON;
			_systemOn = true;
		}
		else {
			_systemState = SYSTEM_OFF;
			_systemOn = false;
		}
	}

	// System state management
	SystemState getSystemState() const { return _systemState; }
	void setSystemState(SystemState state) {
		_systemState = state;
		_systemOn = (state == SYSTEM_ON);
	}
	bool isSystemOff() const { return _systemState == SYSTEM_OFF; }
	bool isSystemStandby() const { return _systemState == SYSTEM_STANDBY; }

	bool isPowerLossMemory() const { return _powerLossMemory; }
	void setPowerLossMemory(bool memory) { _powerLossMemory = memory; }

	// Component state
	bool isHeaterOn() const { return _heaterOn; }
	void setHeaterOn(bool on) { _heaterOn = on; }

	bool isFanOn() const { return _fanOn; }
	void setFanOn(bool on) { _fanOn = on; }

	// Operation mode
	OperationMode getMode() const { return _mode; }
	void setMode(OperationMode mode) { _mode = mode; }

	// Control mode
	ControlMode getControlMode() const { return _controlMode; }
	void setControlMode(ControlMode mode) {
#if GPIO_INTERFACE_ENABLED
		_controlMode = mode;
		if (mode == CONTROL_AUTO_TEMP || mode == CONTROL_AUTO_HUM) _mode = DRY_MODE_BY_HUM;
#else
		// When GPIO is disabled, only USER mode is allowed
		_controlMode = mode;
		//_controlMode = CONTROL_USER_TEMP;
#endif
	}

	// Setpoints
	uint8_t getTargetTemp() const { return _targetTemp; }
	void setTargetTemp(uint8_t temp) { _targetTemp = temp; }

	uint8_t getTargetHumidity() const { return _targetHumidity; }
	void setTargetHumidity(uint8_t hum) { _targetHumidity = hum; }

	// Drying time
	uint32_t getDryDuration() const { return _dryDuration; }
	void setDryDuration(uint32_t duration) { _dryDuration = duration; }

	uint32_t getDryStartTime() const { return _dryStartTime; }
	void setDryStartTime(uint32_t time) { _dryStartTime = time; }

	uint32_t getDryStartTimer() const { return _dryStartTimer; }
	void setDryStartTimer(uint32_t timer) { _dryStartTimer = timer; }

	uint32_t getDryReStartTimer() const { return _dryReStartTimer; }
	void setDryReStartTimer(uint32_t restartTimer) { _dryReStartTimer = restartTimer; }

	bool isManualTurnedOff() const { return _turnedoff; }
	void setManualTurnedOff(bool turnedOff) { _turnedoff = turnedOff; }

	uint8_t getScreenTimeout() const { return _ScreenSaverTimeout; }
	void setScreenTimeout(uint8_t timeout) { _ScreenSaverTimeout = timeout; }

	// Current view
	ViewID getCurrentView() const { return _currentView; }
	void setCurrentView(ViewID view) { _currentView = view; }

	// Edit mode
	bool isEditing() const { return _editing; }
	void setEditing(bool editing) { _editing = editing; }



	// Print state
	bool isPrinting() const { return _printing; }
	void setPrintingState(bool printing) { _printing = printing; }

	// Remaining time in DRY mode (seconds)
	uint32_t getRemainingDryTime() const;

private:
	bool _systemOn = false;
	bool _heaterOn = false;
	bool _fanOn = false;
	bool _printing = false;
	bool _turnedoff = true;
	OperationMode _mode = DRY_MODE_BY_HUM;
#if GPIO_INTERFACE_ENABLED
	ControlMode _controlMode = CONTROL_AUTO_TEMP;
#else
	ControlMode _controlMode = CONTROL_USER_TEMP;
#endif
	uint8_t _targetTemp = DEFAULT_TEMP;
	uint8_t _targetHumidity = DEFAULT_HUM;
	uint32_t _dryDuration = 60;  // Default 60 minutes
	uint32_t _dryStartTime = 0;  // Timestamp when drying started
	uint32_t _dryStartTimer = 0;
	uint32_t _dryReStartTimer = 0;
	uint8_t _ScreenSaverTimeout = 1; // Default 1 minute
	ViewID _currentView = ViewID::STANDBY;
	bool _editing = false;
	bool _powerLossMemory = false; // Added for power loss memory feature
	SystemState _systemState = SYSTEM_OFF; // Added for system state management
};

// Global instance
extern StateManager stateManager;
void saveSystemStateManagerEEPROM(SystemState state);

#endif // STATE_MANAGER_H 