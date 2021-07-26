/*
\brief Reading temperature from ds18b20 sensor
*/


#ifndef DS18B20_H_
#define DS18B20_H_

#include <stdint.h>

#define SENSOR_NO	0xfa00				//Sensor not found or not responding. A known impossible temperature value has been selected.

uint16_t GetTemperDS18b20(uint8_t attempt);			//returns the two's complement temperature value

#endif /* DS18B20_H_ */