#ifndef PID_MANAGER_H
#define PID_MANAGER_H

#include <GyverPID.h>
#include "../core/system_config.h"
#include "../sensors/sensors.h"

// PID instance for heater control
extern GyverPID heaterPID;

void setupPID();
void pidTask();
uint8_t updateHeaterControl();

#endif // PID_MANAGER_H 