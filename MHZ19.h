/*
 * driver for MH-Z19 CO2 sensor
 *
 */ 


#ifndef MHZ19_H_
#define MHZ19_H_

#include <stdint.h>
#include <stdbool.h>

/*
\brief starts the sensor warm-up and waits for it to be ready. 
If readiness is not reached within the timeout, then returns false.
*/
bool MHZ19_ready(uint8_t attempt);
/*
\brief stop sensor, power off
*/
void MHZ19_stop(void);
/*
\brief returns the two's complement CO2 ppm value
returns 0 if sensor broken
*/
uint16_t GetCO2_MHZ19(uint8_t attempt);
#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
bool usart_is_busy(void);
#endif

#endif /* MHZ19_H_ */