#ifndef PRINT_STATUS_SENSOR_H
#define PRINT_STATUS_SENSOR_H

#include <Arduino.h>
#include "../core/system_config.h"

class PrintStatusSensor {
public:
    PrintStatusSensor();
    void begin();
    bool isPrinting() const;
    
private:
    mutable bool lastPrintState;
};

extern PrintStatusSensor printStatusSensor;

#endif // PRINT_STATUS_SENSOR_H 