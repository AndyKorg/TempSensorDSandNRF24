/*
 * driver for MH-Z19 CO2 sensor
 *
 */ 


#ifndef MHZ19_H_
#define MHZ19_H_

#include <stdint.h>
#include <stdbool.h>

uint16_t GetCO2_MHZ19(uint8_t attempt);			//returns the two's complement CO2 ppm value
#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
bool usart_is_busy(void);
#endif

#endif /* MHZ19_H_ */