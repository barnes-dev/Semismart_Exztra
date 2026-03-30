#include "../../include/sensors/print_status_sensor.h"

PrintStatusSensor printStatusSensor;

PrintStatusSensor::PrintStatusSensor() : lastPrintState(false) {}

void PrintStatusSensor::begin() {
    pinMode(PIN_PRINT_STATUS, INPUT);
    lastPrintState = digitalRead(PIN_PRINT_STATUS);
}

bool PrintStatusSensor::isPrinting() const {
    bool currentState = digitalRead(PIN_PRINT_STATUS);
    if (currentState != lastPrintState) lastPrintState = currentState;
    return lastPrintState;
} 