#ifndef THERMAL_SECURITY_H
#define THERMAL_SECURITY_H

#include <Arduino.h>
#include "../core/system_config.h"

enum ThermalSecurityState {
    THERMAL_NORMAL,           // Normal state, no issues
    THERMAL_PROTECTION,       // Protection active, heater off
    THERMAL_COOLDOWN          // Post-protection cooldown, fan on
};

struct ThermalSecuritySystem {
    ThermalSecurityState state;
    uint32_t stateStartTime;
    float lastValidTemperature;
    ThermalSecuritySystem();
    void init();
    bool evaluateSafety(float currentTemp);
    ThermalSecurityState getState() const { return state; }
    bool isHeaterAllowed() const { return (state == THERMAL_NORMAL); }
    bool isCooldownRequired() const { return (state == THERMAL_PROTECTION || state == THERMAL_COOLDOWN); }
private:
    bool validateTemperatureReading(float temp);
    bool handleNormalState(float currentTemp, bool isValidReading, uint32_t currentTime);
    bool handleProtectionState(float currentTemp, bool isValidReading, uint32_t currentTime);
    bool handleCooldownState(float currentTemp, bool isValidReading, uint32_t currentTime);
    void setState(ThermalSecurityState newState, uint32_t currentTime);
};

extern ThermalSecuritySystem thermalSecurity;

inline void initThermalSecurity() { thermalSecurity.init(); }
inline bool evaluateThermalSafety(float temperature) { return thermalSecurity.evaluateSafety(temperature); }
inline bool isHeaterAllowed() { return thermalSecurity.isHeaterAllowed(); }
inline bool isCooldownRequired() { return thermalSecurity.isCooldownRequired(); }
inline ThermalSecurityState getThermalSecurityState() { return thermalSecurity.getState(); }

void updateThermalSecurity();

#endif // THERMAL_SECURITY_H 