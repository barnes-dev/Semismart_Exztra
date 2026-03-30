#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <stdint.h>

// Pin definitions for the potentiometers used to simulate the sensors on a breadboard for testing.
#define MOC_HUM_PIN A3
#define MOC_TEMP_PIN A6

class SensorManager {
public:
	void begin();
	void update();

	// Comment out the following two line to use potentiomenters on a breadboard for testing.
	// This will allow you to simulate the temperature and humidity sensors by turning the potentiometers, 
	// which will change the raw ADC values read by analogRead() and thus change the calculated temperature 
	// and humidity values returned by these functions.
	
	float getTemperature() const { return _temperature; }
	float getHumidity() const { return _humidity; }

	// To test the setup on a breadboard using potent	iometers to simulate the temperature and humidity sensors, 
	// you can use the following code to convert the raw ADC values to temperature and humidity.
	// Use inputs A0 for humidity and A1 for temperature. 
	// This assumes a linear relationship where 0-1023 corresponds to 0-5V, and that the sensors output 10mV per degree 
	// Celsius for temperature and 10mV per %RH for humidity. 
	// Adjust the conversion factors as needed based on your specific sensors.

//	float getTemperature() const { return analogRead(MOC_TEMP_PIN) * (1.0 / 1024.0) * 75.0; }  // Convert raw ADC to temperature (assuming 10mV/°C and 5V reference)
//	float getHumidity() const { return analogRead(MOC_HUM_PIN) * (1.0 / 1024.0) * 70.0; }  // Convert raw ADC to humidity (assuming 10mV/%RH and 5V reference)


	bool isValid() const { return _valid; }
	bool isSHT4Available() const { return _sht4Available; }
	void updateTemperature(float temp, int16_t tempRaw);
	void updateHumidity(float hum, int16_t humRaw);
	void setValid(bool valid);
private:
	float _temperature = 25.0f;
	float _humidity = 50.0f;
	int16_t _temperatureRaw = 250;  // 25.0°C * 10
	int16_t _humidityRaw = 500;     // 50.0% * 10
	bool _valid = true;
	bool _sht4Available = false;
};

extern SensorManager sensorManager;

#endif // SENSOR_MANAGER_H 