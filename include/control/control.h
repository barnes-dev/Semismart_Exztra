#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "../core/system_config.h"
#include "../sensors/sensors.h"
#include "../safety/thermal_security.h"
#include "../../include/control/pid_manager.h"

// Fan states
enum FanState : uint8_t {
    FAN_OFF,
    FAN_COOLDOWN,
    FAN_STANDBY_CYCLING_ON,
    FAN_STANDBY_CYCLING_OFF,
    FAN_STANDBY_OFF,        // New state for complete OFF in standby
    FAN_ON
};

struct ControlSystem {
    FanState fanState;
    uint32_t stateStartTime;
    bool heaterPrevState;
    int heaterPower; // Heater power
    bool isStandby;
    uint32_t lastThermalCheck;
    
    // Fan smooth transition variables
    uint8_t currentFanSpeed;
    uint8_t targetFanSpeed;
    uint32_t fanTransitionStartTime;
    bool fanTransitionActive;
    
    ControlSystem();
    
    void init();
    void setHeaterPower(int power);
    void updateFanState();
    void setFanState(FanState newState);
    void handleSystemShutdown();
    void setStandbyMode(bool standby);
    bool isFanCooldownActive() const;
    bool isFanOn() const;
    bool isHeaterOn() const;
    FanState getFanState() const;
    bool isCooldownRequired() const { return (getThermalSecurityState() == THERMAL_PROTECTION); }
    
    void setFanSpeedWithTransition(uint8_t targetSpeed);
    void updateFanTransition();
    uint8_t getCurrentFanSpeed() const;
};

extern ControlSystem controlSystem;

inline void initControl() { controlSystem.init(); }
inline void handleSystemShutdown() { controlSystem.handleSystemShutdown(); }
inline void setStandbyMode(bool standby) { controlSystem.setStandbyMode(standby); }
inline bool isFanCooldownActive() { return controlSystem.isFanCooldownActive(); }
inline bool isFanOn() { return controlSystem.isFanOn(); }
inline bool isHeaterOn() { return controlSystem.isHeaterOn(); }
inline FanState getFanState() { return controlSystem.getFanState(); }
inline uint8_t getCurrentFanSpeed() { return controlSystem.getCurrentFanSpeed(); }

#endif // CONTROL_H 