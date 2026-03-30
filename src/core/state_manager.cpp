#include "../../include/core/state_manager.h"
#include "../../include/utils/storage.h"
#include <Arduino.h>

StateManager stateManager;

void StateManager::begin() {
	// Load and validate values from EEPROM
	_targetTemp = readTargetTempEEPROM();
	if (_targetTemp < TEMP_MIN || _targetTemp > TEMP_MAX) _targetTemp = DEFAULT_TEMP;

	_targetHumidity = readTargetHumEEPROM();
	if (_targetHumidity < HUM_MIN || _targetHumidity > HUM_MAX) _targetHumidity = DEFAULT_HUM;

	_mode = (OperationMode)readModeEEPROM();
	if (_mode != DRY_MODE_BY_HUM && _mode != DRY_MODE_BY_TIME) _mode = DRY_MODE_BY_HUM;

	_dryDuration = readDryDurationEEPROM();
	if (_dryDuration < DRY_DURATION_MIN || _dryDuration > DRY_DURATION_MAX) _dryDuration = DRY_DURATION_DEFAULT;

	_dryStartTimer = readDryStartTimeEEPROM();
	if (_dryStartTimer > DRY_START_MAX) _dryStartTimer = DRY_START_MIN;

	_controlMode = (ControlMode)readControlModeEEPROM();

	_powerLossMemory = readPowerLossMemoryEEPROM();

	_ScreenSaverTimeout = readScreenTimeoutEEPROM();

#if GPIO_INTERFACE_ENABLED
	if (_controlMode != CONTROL_USER_TEMP && _controlMode != CONTROL_USER_HUM && _controlMode
		!= CONTROL_AUTO_TEMP && _controlMode != CONTROL_AUTO_HUM)
		_controlMode = CONTROL_AUTO_TEMP;
#else
	// When GPIO is disabled, force USER control mode
	if (_controlMode != CONTROL_USER_TEMP && _controlMode != CONTROL_USER_HUM && _controlMode
		!= CONTROL_AUTO_TEMP && _controlMode != CONTROL_AUTO_HUM)
		_controlMode = CONTROL_USER_TEMP;
#endif

	// Force BY_HUM mode for AUTO control mode
	if (_controlMode == CONTROL_AUTO_TEMP || _controlMode == CONTROL_AUTO_HUM) _mode = DRY_MODE_BY_HUM;

	// Initialize system state - start in OFF state
	_systemState = SYSTEM_OFF;
	_systemOn = false;
}

uint32_t StateManager::getRemainingDryTime() const {
	if (!_systemOn || _mode != DRY_MODE_BY_TIME || _dryStartTime == 0) return 0;
	uint32_t elapsed = millis() - _dryStartTime;
	uint32_t total = _dryDuration * MINUTES_TO_MS;
	return (elapsed >= total) ? 0 : (total - elapsed) / MS_TO_SECONDS;
}