#include "../../include/control/pid_manager.h"
#include "../../include/core/state_manager.h"
#include "../../include/sensors/sensor_manager.h"
#include "../../include/control/control.h"
#include <Arduino.h>

// PID instance for heater control
GyverPID heaterPID(PID_KP, PID_KI, PID_KD);

extern StateManager stateManager;
extern SensorManager sensorManager;
uint8_t power = 0;

void setupPID() {
    heaterPID.setDirection(PID_DIRECTION);
    heaterPID.setLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    heaterPID.setpoint = stateManager.getTargetTemp();
}

void pidTask() {
    if (!stateManager.isSystemOn()) {
        controlSystem.setHeaterPower(0);
        return;
    }

    // Cache frequently accessed values
    float currentTemp = sensorManager.getTemperature();
    uint8_t targetTemp = stateManager.getTargetTemp();
    OperationMode mode = stateManager.getMode();
    
    // Both temperatures are in Celsius, no conversion needed
    
    heaterPID.setpoint = targetTemp;
    heaterPID.input = currentTemp;
    heaterPID.getResult();
    
    float finalPower = heaterPID.output;
    
    // Early exit for humidity mode when target reached
    if (mode == DRY_MODE_BY_HUM && sensorManager.getHumidity() <= stateManager.getTargetHumidity()) {
        finalPower = 0;
    }
    
    controlSystem.setHeaterPower(finalPower);
} 

uint8_t updateHeaterControl() {
    float currentTemp = readThermistorTemperature() / 10.0f;

    // HARD SAFETY CUTOFF
    if (currentTemp >= HARD_CUTOFF_TEMP) {
        return 0;
    }

    // FULL POWER ZONE (≤ 50°C)
    if (currentTemp <= FULL_POWER_TEMP) {
        power = 255;
    }
    // TAPER ZONE (50°C → 60°C)
    else if (currentTemp < SECURITY_MAX_HEATER_TEMP) {
        float t = (currentTemp - FULL_POWER_TEMP) /
            (SECURITY_MAX_HEATER_TEMP - FULL_POWER_TEMP);
        // t goes from 0.0 → 1.0

        float p = 255.0f - t * (255.0f - MIN_HEATER_POWER);
        power = (uint8_t)p;
		if (power < MIN_HEATER_POWER) power = MIN_HEATER_POWER; // Ensure we never go below the minimum power in the taper zone
    }
    // HOLD ZONE (exactly 60°C)
    else {
        power = MIN_HEATER_POWER;
    }

    return (uint8_t)power;
}
