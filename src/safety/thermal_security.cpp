#include "../../include/safety/thermal_security.h"
#include "../../include/sensors/sensors.h"

ThermalSecuritySystem thermalSecurity;

ThermalSecuritySystem::ThermalSecuritySystem() :
    state(THERMAL_NORMAL),
    stateStartTime(0),
    lastValidTemperature(0.0) {}

void ThermalSecuritySystem::init() {
    state = THERMAL_NORMAL;
    stateStartTime = millis();
    lastValidTemperature = 0.0;
}

bool ThermalSecuritySystem::evaluateSafety(float currentTemp) {
    uint32_t currentTime = millis();
    bool isValidReading = validateTemperatureReading(currentTemp);
    switch (state) {
        case THERMAL_NORMAL:
            return handleNormalState(currentTemp, isValidReading, currentTime);
        case THERMAL_PROTECTION:
            return handleProtectionState(currentTemp, isValidReading, currentTime);
        case THERMAL_COOLDOWN:
            return handleCooldownState(currentTemp, isValidReading, currentTime);
        default:
            setState(THERMAL_NORMAL, currentTime);
            return true;
    }
}

bool ThermalSecuritySystem::validateTemperatureReading(float temp) {
    return temp != SECURITY_SENSOR_ERROR_VALUE && 
           !isnan(temp) && 
           temp >= THERMAL_SECURITY_TEMP_MIN && 
           temp <= THERMAL_SECURITY_TEMP_MAX && 
           (lastValidTemperature <= 0.0 || (temp - lastValidTemperature) <= SECURITY_SENSOR_MAX_TEMP_CHANGE);
}

bool ThermalSecuritySystem::handleNormalState(float currentTemp, bool isValidReading, uint32_t currentTime) {
    if (!isValidReading) return true;
    if (currentTemp >= SECURITY_PROTECTION_THRESHOLD) {
        setState(THERMAL_PROTECTION, currentTime);
        lastValidTemperature = currentTemp;
        return false;
    }
    lastValidTemperature = currentTemp;
    return true;
}

bool ThermalSecuritySystem::handleProtectionState(float currentTemp, bool isValidReading, uint32_t currentTime) {
    uint32_t protectionDuration = currentTime - stateStartTime;
    if (isValidReading) {
        lastValidTemperature = currentTemp;
        if (protectionDuration >= SECURITY_PROTECTION_TIME) {
            if (currentTemp <= SECURITY_RECOVERY_THRESHOLD) {
                setState(THERMAL_COOLDOWN, currentTime);
                return false;
            }
        }
    }
    return false;
}

bool ThermalSecuritySystem::handleCooldownState(float currentTemp, bool isValidReading, uint32_t currentTime) {
    uint32_t cooldownDuration = currentTime - stateStartTime;
    if (isValidReading) {
        lastValidTemperature = currentTemp;
        if (currentTemp >= SECURITY_PROTECTION_THRESHOLD) {
            setState(THERMAL_PROTECTION, currentTime);
            return false;
        }
        if (cooldownDuration >= SECURITY_COOLDOWN_TIME) {
            setState(THERMAL_NORMAL, currentTime);
            return true;
        }
    }
    return false;
}

void ThermalSecuritySystem::setState(ThermalSecurityState newState, uint32_t currentTime) {
    if (state != newState) state = newState, stateStartTime = currentTime;
} 

void updateThermalSecurity() {
    float heaterTemp = readThermistorTemperature() / 10.0f;
    thermalSecurity.evaluateSafety(heaterTemp);
} 