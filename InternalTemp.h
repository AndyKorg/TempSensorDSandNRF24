/*
\brief internal temperature sensor driver MK
 */

#ifndef INTERNALTEMP_H_
#define INTERNALTEMP_H_

#include <stdint.h>

uint16_t GetTemperature(uint8_t attempt); //returns the two's complement temperature value

#endif /* INTERNALTEMP_H_ */
