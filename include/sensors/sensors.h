#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <stdint.h>

void initSensors();
void readSensors();
bool validateTemperatureReading(int16_t temp);
int16_t readThermistorTemperature();

#endif // SENSORS_H 