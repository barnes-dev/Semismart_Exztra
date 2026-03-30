#include "../../include/sensors/sensor_manager.h"
#include "../../include/sensors/sensors.h"

SensorManager sensorManager;

void SensorManager::begin() {
	initSensors();
	update();
}

void SensorManager::update() {
	readSensors();
	extern bool sht4Available;
	_sht4Available = sht4Available;
}

void SensorManager::updateTemperature(float temp, int16_t tempRaw) {
	_temperature = temp;
	_temperatureRaw = tempRaw;
}

void SensorManager::updateHumidity(float hum, int16_t humRaw) {
	_humidity = hum;
	_humidityRaw = humRaw;
}

void SensorManager::setValid(bool valid) {
	_valid = valid;
}