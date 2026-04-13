#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "../core/system_config.h"

#if ARDUINO_ESP32

#include <Preferences.h>

inline Preferences MAIN_EEPROM;

#define EEPROM_VERSION 2

inline void initStorage() {
	MAIN_EEPROM.begin("main_storage", false);

	if (MAIN_EEPROM.getUInt("ADDR_VERSION", 0) != EEPROM_VERSION) {
		MAIN_EEPROM.getUInt("TEMPERATURE", DEFAULT_TEMP);
		MAIN_EEPROM.getUInt("HUMIDITY", DEFAULT_HUM);
		MAIN_EEPROM.putUInt("MODE", DRY_MODE_BY_HUM);
		MAIN_EEPROM.putULong("DRY_DURATION", (uint32_t)60);
		MAIN_EEPROM.putUInt("DRY_START_TIME", DRY_START_MIN);
#if GPIO_INTERFACE_ENABLED
		MAIN_EEPROM.putUInt("CONTROL_MODE", CONTROL_AUTO_TEMP);
#else
		MAIN_EEPROM.putUInt("CONTROL_MODE", CONTROL_USER_TEMP);
#endif
		MAIN_EEPROM.putUInt("ADDR_VERSION", EEPROM_VERSION);
		MAIN_EEPROM.putUInt("SYSTEM_STATE", 0); // Initialize system state if needed  
		MAIN_EEPROM.putUInt("POWER_LOSS_MEM", 0); // Initialize power loss memory if needed
		MAIN_EEPROM.putUInt("SCR_TIMEOUT", 1); // Initialize screen timeout if needed         
	}
	MAIN_EEPROM.end();
}

inline uint8_t readTargetTempEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_temp = MAIN_EEPROM.getUInt("TEMPERATURE", 0); MAIN_EEPROM.end(); return eeprom_temp; }
inline uint8_t readTargetHumEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_hum = MAIN_EEPROM.getUInt("HUMIDITY", 0); MAIN_EEPROM.end(); return eeprom_hum; }
inline uint8_t readModeEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_mode = MAIN_EEPROM.getUInt("MODE", 0); MAIN_EEPROM.end(); return eeprom_mode; }
inline uint32_t readDryDurationEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint32_t eeprom_duration = MAIN_EEPROM.getULong("DRY_DURATION", 0); MAIN_EEPROM.end(); return eeprom_duration; }
inline uint32_t readDryStartTimeEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint32_t eeprom_time = MAIN_EEPROM.getULong("DRY_START_TIME", 0); MAIN_EEPROM.end(); return eeprom_time; }
inline uint8_t readControlModeEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_control_mode = MAIN_EEPROM.getUInt("CONTROL_MODE", 0); MAIN_EEPROM.end(); return eeprom_control_mode; }
inline uint8_t readSystemStateEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_system_state = MAIN_EEPROM.getUInt("SYSTEM_STATE", 0); MAIN_EEPROM.end(); return eeprom_system_state; }
inline uint8_t readPowerLossMemoryEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_power_loss_memory = MAIN_EEPROM.getUInt("POWER_LOSS_MEM", 0); MAIN_EEPROM.end(); return eeprom_power_loss_memory; }
inline uint8_t readScreenTimeoutEEPROM() { MAIN_EEPROM.begin("main_storage", true); uint8_t eeprom_screen_timeout = MAIN_EEPROM.getUInt("SCR_TIMEOUT", 0); MAIN_EEPROM.end(); return eeprom_screen_timeout; }

inline void saveTargetTempEEPROM(uint8_t temp) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("TEMPERATURE", temp); MAIN_EEPROM.end(); }
inline void saveTargetHumEEPROM(uint8_t hum) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("HUMIDITY", hum); MAIN_EEPROM.end(); }
inline void saveModeEEPROM(uint8_t mode) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("MODE", mode); MAIN_EEPROM.end(); }
inline void saveDryDurationEEPROM(uint32_t duration) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putULong("DRY_DURATION", duration); MAIN_EEPROM.end(); }
inline void saveDryStartTimeEEPROM(uint32_t time) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putULong("DRY_START_TIME", time); MAIN_EEPROM.end(); }
inline void saveControlModeEEPROM(uint8_t mode) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("CONTROL_MODE", mode); MAIN_EEPROM.end(); }
inline void saveSystemStateEEPROM(uint8_t state) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("SYSTEM_STATE", state); MAIN_EEPROM.end(); }
inline void savePowerLossMemoryEEPROM(uint8_t memory) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("POWER_LOSS_MEM", memory); MAIN_EEPROM.end(); }
inline void saveScreenTimeoutEEPROM(uint8_t timeout) { MAIN_EEPROM.begin("main_storage", false); MAIN_EEPROM.putUInt("SCR_TIMEOUT", timeout); MAIN_EEPROM.end(); }

#else

#include <EEPROM.h>

#define EEPROM_ADDR_VERSION 0
#define EEPROM_ADDR_TEMP 1
#define EEPROM_ADDR_HUM 2
#define EEPROM_ADDR_MODE 3
#define EEPROM_ADDR_DRY_DURATION 4
#define EEPROM_ADDR_CONTROL_MODE 8
#define EEPROM_ADDR_DRY_START_TIME 12
#define EEPROM_ADDR_SYSTEM_STATE 16  // Starting address for system state storage (if needed in the future)
#define EEPROM_ADDR_SCREEN_TIMEOUT 18 // Address to store screen timeout (if needed in the future)
#define EEPROM_POWER_LOSS_MEMORY 20 // Address to store power loss memory (if needed in the future)

#define EEPROM_VERSION 2

inline void initStorage() {
	if (EEPROM.read(EEPROM_ADDR_VERSION) != EEPROM_VERSION) {
		EEPROM.write(EEPROM_ADDR_TEMP, DEFAULT_TEMP);
		EEPROM.write(EEPROM_ADDR_HUM, DEFAULT_HUM);
		EEPROM.write(EEPROM_ADDR_MODE, DRY_MODE_BY_HUM);
		EEPROM.put(EEPROM_ADDR_DRY_DURATION, (uint32_t)60);
		EEPROM.put(EEPROM_ADDR_DRY_START_TIME, DRY_START_MIN);
#if GPIO_INTERFACE_ENABLED
		EEPROM.write(EEPROM_ADDR_CONTROL_MODE, CONTROL_AUTO_TEMP);
#else
		EEPROM.write(EEPROM_ADDR_CONTROL_MODE, CONTROL_USER_TEMP);
#endif
		EEPROM.write(EEPROM_ADDR_VERSION, EEPROM_VERSION);
		EEPROM.write(EEPROM_ADDR_SYSTEM_STATE, 0); // Initialize system state if needed  
		EEPROM.write(EEPROM_POWER_LOSS_MEMORY, 0); // Initialize power loss memory if needed
		EEPROM.write(EEPROM_ADDR_SCREEN_TIMEOUT, 1); // Initialize screen timeout if needed         
	}
}

inline uint8_t readTargetTempEEPROM() { return EEPROM.read(EEPROM_ADDR_TEMP); }
inline uint8_t readTargetHumEEPROM() { return EEPROM.read(EEPROM_ADDR_HUM); }
inline uint8_t readModeEEPROM() { return EEPROM.read(EEPROM_ADDR_MODE); }
inline uint32_t readDryDurationEEPROM() { uint32_t duration; EEPROM.get(EEPROM_ADDR_DRY_DURATION, duration); return duration; }
inline uint32_t readDryStartTimeEEPROM() { uint32_t time; EEPROM.get(EEPROM_ADDR_DRY_START_TIME, time); return time; }
inline uint8_t readControlModeEEPROM() { return EEPROM.read(EEPROM_ADDR_CONTROL_MODE); }
inline uint8_t readSystemStateEEPROM() { return EEPROM.read(EEPROM_ADDR_SYSTEM_STATE); }
inline uint8_t readPowerLossMemoryEEPROM() { return EEPROM.read(EEPROM_POWER_LOSS_MEMORY); }
inline uint8_t readScreenTimeoutEEPROM() { return EEPROM.read(EEPROM_ADDR_SCREEN_TIMEOUT); }

inline void saveTargetTempEEPROM(uint8_t temp) { EEPROM.write(EEPROM_ADDR_TEMP, temp); }
inline void saveTargetHumEEPROM(uint8_t hum) { EEPROM.write(EEPROM_ADDR_HUM, hum); }
inline void saveModeEEPROM(uint8_t mode) { EEPROM.write(EEPROM_ADDR_MODE, mode); }
inline void saveDryDurationEEPROM(uint32_t duration) { EEPROM.put(EEPROM_ADDR_DRY_DURATION, duration); }
inline void saveDryStartTimeEEPROM(uint32_t time) { EEPROM.put(EEPROM_ADDR_DRY_START_TIME, time); }
inline void saveControlModeEEPROM(uint8_t mode) { EEPROM.write(EEPROM_ADDR_CONTROL_MODE, mode); }
inline void saveSystemStateEEPROM(uint8_t state) { EEPROM.write(EEPROM_ADDR_SYSTEM_STATE, state); }
inline void savePowerLossMemoryEEPROM(uint8_t memory) { EEPROM.write(EEPROM_POWER_LOSS_MEMORY, memory); }
inline void saveScreenTimeoutEEPROM(uint8_t timeout) { EEPROM.write(EEPROM_ADDR_SCREEN_TIMEOUT, timeout); }

#endif // ARDUINO_NANO_R4 

#endif // STORAGE_H 