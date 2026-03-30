#include "../../include/control/pid_manager.h"
#include "../../include/core/state_manager.h"
#include "../../include/sensors/sensor_manager.h"
#include "../../include/control/control.h"
#include <Arduino.h>

// PID instance for heater control
GyverPID heaterPID(PID_KP, PID_KI, PID_KD);

extern StateManager stateManager;
extern SensorManager sensorManager;

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