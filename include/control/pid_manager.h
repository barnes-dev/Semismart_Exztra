#ifndef PID_MANAGER_H
#define PID_MANAGER_H

#include <GyverPID.h>
#include "../core/system_config.h"

// PID instance for heater control
extern GyverPID heaterPID;

void setupPID();
void pidTask();

#endif // PID_MANAGER_H 