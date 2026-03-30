#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "../core/system_config.h"

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

#endif // STORAGE_H 