/*
 * 1-wire bus
 */ 


#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_

#include <stdint.h>
#include <stdbool.h>

//------------- command 1-Ware bus
#define ONE_WARE_SEARCH_ROM		0xf0	//search devices
#define ONE_WARE_READ_ROM		0x33	//read one devices
#define ONE_WARE_MATCH_ROM		0x55	//select devices
#define ONE_WARE_SKIP_ROM		0xcc	//skip address

typedef bool (*one_wire_cb)(uint8_t *value);

void OneWareIni(void);
//Reset the bus and wait for a response from the device. In the answer_cb function, the parameter value = 0 if the device does not answer, 0xff - if the device is on the bus.
void OneWareReset(one_wire_cb answer_cb);
void OneWareSendByte(uint8_t SendByte, one_wire_cb send_end_cb);
uint8_t OneWareReciveByte(void);

#endif /* ONE_WIRE_H_ */