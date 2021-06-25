/*
\brief timers not used!
 */ 


#ifndef ONEWARE_H_
#define ONEWARE_H_

#include <stdint.h>

//------------- Command 1-Ware
#define ONE_WARE_SEARCH_ROM		0xf0								//Address search - used for a universal algorithm for determining the number and addresses of connected devices
#define ONE_WARE_READ_ROM		0x33								//Read device address - used to determine the address of the only device on the bus
#define ONE_WARE_MATCH_ROM		0x55								//Address selection - used to refer to a specific device address from many connected
#define ONE_WARE_SKIP_ROM		0xcc								//Ignore address - used to address all devices at once, while the device address is ignored (you can access an unknown device)

void OneWareIni(void);
uint8_t OneWareReset(void);											//Reset the bus and wait for a response from the device. 1 - there is a device on the bus, 0 - no one answered
void OneWareSendByte(uint8_t SendByte);								//Send a byte over the bus
uint8_t OneWareReciveByte(void);									//Receive a byte on the bus

#endif /* ONEWARE_H_ */