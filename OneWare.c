/*
\brief timers not used!
*/


#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <stdbool.h>
#include "HAL.h"
#include <util/delay.h>
#include "OneWare.h"

#define ONE_WARE_RESET_480_us		480								//������������ �������� Reset �� ���� 1-Ware 480 ��� �������, ������ 560 ��� � samsung ���������
#define ONE_WARE_2_us				2								//���������� ����� �����������, ������ 1 ���, ����� �� 2 ��� �� ������ ������
#define ONE_WARE_60_us				60								//������������ ������������ ����-�����
#define ONE_WARE_240_us				240								//������������ ������������ �������� ������ �� Reset
#define ONE_WARE_14_us				14								//����� ������� ������� ��������� ���� �� ����������
#define ONE_WARE_5_us				5								//������������ ������������ ������� ����� ��������� ������

#define ONE_WIRE_TIC(x)				F_CPU/(1000000UL/(x+5))

#define ONE_WIRE_NULL()				do {ONE_WIRE_PORT.DIRSET = ONE_WIRE_PIN;} while (0)	//pin set down
#define ONE_WIRE_ONE_REL()			do {ONE_WIRE_PORT.DIRCLR = ONE_WIRE_PIN;} while (0)	//pin release
#define ONE_WIRE_IS_NULL()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?0:1)	//input low level
#define ONE_WIRE_IS_ONE()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?1:0)	//input hight level

/*
\brief reset bus and check device on the bus
*/
uint8_t OneWareReset(void){
	uint8_t Ret = 0;
	
	bool i_state = isr_state();

	cli();
	//enable tightening during tire testing
	ONE_WIRE_PORT.ONE_WIRE_PIN_CTRL |= PORT_PULLUPEN_bm;
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
	ONE_WIRE_PORT.ONE_WIRE_PIN_CTRL &= ~PORT_PULLUPEN_bm;
	_delay_us(ONE_WARE_RESET_480_us);	//delay between command
	if (i_state) sei();
	return Ret;
}

/*
\brief send byte from one device
*/
void OneWareSendByte(uint8_t SendByte){
	uint8_t Count;
	bool i_state = isr_state();

	cli();
	for(Count=8; Count; Count--){
		ONE_WIRE_NULL();
		_delay_us(ONE_WARE_2_us);	//start
		if (SendByte & 1) ONE_WIRE_ONE_REL();
		SendByte >>= 1;
		_delay_us(ONE_WARE_60_us);	//slot
		ONE_WIRE_ONE_REL();
		_delay_us(ONE_WARE_2_us);	//delay next slot
	}
	if (i_state) sei();
}

/*
\brief read byte from one device
*/
uint8_t OneWareReciveByte(void){
	uint8_t Count, Ret = 0;
	bool i_state = isr_state();

	cli();
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
	if (i_state) sei();
	return Ret;
}

void OneWareIni(void){
	ONE_WIRE_PORT.OUTCLR = ONE_WIRE_PIN;
	ONE_WIRE_NULL();
}