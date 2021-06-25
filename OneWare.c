/*
\brief timers not used!
*/


#include <stdint.h>
#include <avr/io.h>
#include "HAL.h"
#include <util/delay.h>
#include "OneWare.h"

#define ONE_WARE_RESET_480_us		480								//Длительность импульса Reset на шине 1-Ware 480 мкс минимум, сделаю 560 как у samsung протокола
#define ONE_WARE_2_us				2								//Промежуток между таймслотами, обычно 1 мкс, довел до 2 мкс на всякий случай
#define ONE_WARE_60_us				60								//Максимальная длительность тайм-слота
#define ONE_WARE_240_us				240								//Максимальная длительность ожидания ответа на Reset
#define ONE_WARE_14_us				14								//Конец периода анализа ответного бита от устройства
#define ONE_WARE_5_us				5								//Максимальная длительность периода после таймслота чтения

#define ONE_WIRE_NULL()				do {ONE_WIRE_PORT.DIRSET = ONE_WIRE_PIN;} while (0)	//pin set down
#define ONE_WIRE_ONE_REL()			do {ONE_WIRE_PORT.DIRCLR = ONE_WIRE_PIN;} while (0)	//pin release
#define ONE_WIRE_IS_NULL()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?0:1)	//input low level
#define ONE_WIRE_IS_ONE()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?1:0)	//input hight level


/*
\brief reset bus and check device on the bus
*/
uint8_t OneWareReset(void){
	uint8_t Ret = 0;
	
	ONE_WIRE_NULL();
	_delay_us(ONE_WARE_RESET_480_us);	//presence
	ONE_WIRE_ONE_REL();
	_delay_us(ONE_WARE_14_us);			//wait device answer
	for(uint8_t i=(ONE_WARE_240_us/ONE_WARE_2_us); i; i--){
		_delay_us(ONE_WARE_2_us);		//or timeout
		if ONE_WIRE_IS_NULL(){			//or answer
			Ret = 1;
			break;
		}
	}
	_delay_us(ONE_WARE_RESET_480_us);	//delay between command
	return Ret;
}

/*
\brief send byte from one device
*/
void OneWareSendByte(uint8_t SendByte){
	uint8_t Count;

	for(Count=8; Count; Count--){
		ONE_WIRE_NULL();
		_delay_us(ONE_WARE_2_us);	//start
		if (SendByte & 1) ONE_WIRE_ONE_REL();
		SendByte >>= 1;
		_delay_us(ONE_WARE_60_us);	//slot
		ONE_WIRE_ONE_REL();
		_delay_us(ONE_WARE_2_us);	//delay next slot
	}
}

/*
\brief read byte from one device
*/
uint8_t OneWareReciveByte(void){
	uint8_t Count, Ret = 0;

	for(Count=0; Count<8; Count++){
		ONE_WIRE_NULL();
		_delay_us(ONE_WARE_2_us);	//start
		ONE_WIRE_ONE_REL();
		_delay_us(ONE_WARE_5_us);	// wait
		Ret >>= 1;					// next bit
		if ONE_WIRE_IS_ONE() Ret |= 0b10000000;
		_delay_us(ONE_WARE_60_us);	//slot
		_delay_us(ONE_WARE_2_us);	//delay next slot
	}
	return Ret;
}

void OneWareIni(void){
	ONE_WIRE_PORT.OUTCLR = ONE_WIRE_PIN;
	ONE_WIRE_NULL();
}