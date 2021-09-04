/*
* Внешний датчик температуры.
* Передает четыре байта. Первый байт - зарезервирован для будущего использования, второй байт - тип датчика, пока только 0x01 - датчик температуры на ATTiny13
* И последние два байта температуры полученных из DS18B20. Если нет датчика температуры то передается SENSOR_NO = (0xfa00) - значение температуры выходящие за предел измерения ds18b20
* Ожидает в ответ длительность периода засыпания. Длительность периода передается в виде трех
* байт. Первый байт это значение прескаллера для таймера WDT
* Второй и третий байты это счетчик срабатываний таймера WDT. Младший байт счетчика передается вперед.
* Если передача не удалась то используется период засыпания по умолчанию
* Пример кода на приемнике:
* 		BufTx[0] = ATTINY_13A_8S_SLEEP;		//Команда для таймера сна, заснуть на 8 секунд
*		BufTx[1] = 0x00;					//Старший байт счетчика сна
*		BufTx[2] = 0x01;					//Младший байт счетчика
*		nRF_cmd_Write(nRF_W_ACK_PAYLOAD_MASK | 0b00000000, BUF_LEN, BufTx);	//Записать ответ для канала 0
* Andy Korg (c) v.0.1 2014 г.
* http://radiokot.ru/circuit/digital/home/199/
*/


#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "HAL.h"
#include "nRF24L01P.h"
#include "sleep_rtc.h"

#if SENSOR_TYPE == DEVICE_TYPE_DS18B20
#include "ds18b20.h"
#elif SENSOR_TYPE == DEVICE_TYPE_INTER_TEMPR
#include "InternalTemp.h"
#endif


#ifdef DEBUG
#include "usart.h"
#include <stdio.h>
#endif

#define SLEEP_PERIOD_DEFAULT_S	1

#define SLEEP_PERIOD_LONG_M		4		//sleep period if the receiver does not respond after all transmission attempts, minute
#define ATTEMPT_SEND_MAX		20		//Maximum number of attempts to transfer measurement results or registration mode
#define ATTEMPT_READ_SENSOR		10

#define allow_press_button()	do {BUTTON_PORT.INTFLAGS |= BUTTON_PIN; BUTTON_INT = BUTTON_INT_TYPE | PORT_PULLUPEN_bm;} while (0) //allow button press test

#define halt_sleep()			do {\
									period.dim = dd_Min;\
									period.value = UINT16_MAX;\
									allow_press_button();\
									sleep_period_set(period);\
								} while (0)

volatile static uint8_t mode = nrf_send_mode;

ISR(BUTTON_INT_VECT){
	if (BUTTON_PORT.INTFLAGS & BUTTON_PIN){		//test button pressed
		if (!(BUTTON_PORT.IN & BUTTON_PIN)){
			mode = nrf_reg_mode;
			BUTTON_INT = PORT_PULLUPEN_bm;		//interrupt off
			sleep_period_stop();
		}
		//BUTTON_PORT.INTFLAGS |= BUTTON_PIN;		//allowing retry only after attempts have been exhausted. see below.
	}
}

#ifdef DEBUG
bool console_cmd(cmd_t cmd){
	return true;
}
#endif

inline void pin_off_unused(void){
	//PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	//PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;	//MOSI
	//PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;	//MISO
	//PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;	//SCK
	//PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;	//CSN
	//PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;	//CE
	PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;		//IRQ
	PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;

	PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;		//SDA
	PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;		//SCL
	PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;		//TXD

	PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;	//RXD
	#if SENSOR_TYPE != DEVICE_TYPE_DS18B20
	PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;	//1-wire
	#endif
	PORTB.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;

	//PORTC.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;	//button

	#if SENSOR_TYPE != DEVICE_TYPE_MH_Z19s
	PORTC.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;		//power mh-z19
	#endif
	PORTC.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;		//no pin in MK
	PORTC.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;		//no pin in MK

}

int main(void)
{
	//	#include <avr/xmega.h>
	//	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm);

	uint8_t attempt = ATTEMPT_SEND_MAX;
	uint16_t Tempr;
	
	#ifdef DEBUG
	debugPortIni();
	debugBoth();
	usart_init(console_cmd);
	#endif

	nrf_Response_t nRF_Answer;

	BUTTON_PORT.DIRCLR = BUTTON_PIN;
	BUTTON_INT = BUTTON_INT_TYPE | PORT_PULLUPEN_bm;

	RTC_1KHZ_init();
	mode = nrf_send_mode;
	sei();
	
	pin_off_unused();
	nRF_Init();

	period_t period;

	while(1){
		
		uint8_t buf[4];
		#if SENSOR_TYPE == DEVICE_TYPE_DS18B20
		Tempr = GetTemperDS18b20(ATTEMPT_READ_SENSOR);
		#elif SENSOR_TYPE == DEVICE_TYPE_INTER_TEMPR
		Tempr = GetTemperature(ATTEMPT_READ_SENSOR);
		#endif
		DEBUG_LOG("t %d\r", Tempr);
		buf[0] = SENSOR_TYPE;
		buf[1] = 0;
		buf[2] = (uint8_t)(Tempr>>8);
		buf[3] = (uint8_t)Tempr;
		nrf_err_t res_send = nRF_Send(mode, buf, 4, &nRF_Answer);
		period.dim = dd_Sec;
		period.value = SLEEP_PERIOD_DEFAULT_S;
		if (res_send == nRF_OK){
			if (mode == nrf_send_mode){	//data transferred
				period.dim = dd_Min;
				period.value = SLEEP_PERIOD_LONG_M;	//white long delay if answer not correct TODO:Добавить проверку типа команды и датчика
				if (nRF_Answer.Len == DEVICE_ANSWER_LEN){	//response in the measurement transfer mode, we check the correctness of the response and set the sleep period depending on the response
					period.value = *((uint16_t*)(nRF_Answer.Data+DEVICE_ANSWER_NUM_PERIOD));
					attempt = ATTEMPT_SEND_MAX;
				}
				DEBUG_LOG("send OK %d\r", period.value);
				allow_press_button();
				sleep_period_set(period);
				continue;
			}
			//registration mode
			if (nRF_real_address_is_set()){
				//registration completed successfully
				attempt = ATTEMPT_SEND_MAX;
				mode = nrf_send_mode;	//send data
				DEBUG_LOG("reg ok\r");
				continue;	//!!!!!!!!!!!!!!!!!!!!! continue without sleep!
			}
			DEBUG_LOG("OK no reg %d\r", period.value);
		}
		else if (res_send == nRF_ERR_NO_ANSWER){	//reciver not answer
			period.dim = dd_Sec;				//white 1 second
			period.value = SLEEP_PERIOD_DEFAULT_S;
			if (attempt == 1){
				period.dim = dd_Min;
				period.value = SLEEP_PERIOD_LONG_M;
				DEBUG_LOG("reciver no answer\r");
			}
			else {
				DEBUG_LOG("next attempt %ds %d\r", SLEEP_PERIOD_DEFAULT_S, attempt);
			}
		}
		else if(res_send == nRF_ERR_ADDR_NOT_FOUND){
			DEBUG_LOG("real adr not set\r");
			while(1){
				halt_sleep();
				if (mode == nrf_reg_mode){			//registration mode is set, exit from stop
					attempt = ATTEMPT_SEND_MAX;
					DEBUG_LOG("reg start\r");
					break;
				}
			}
		}
		else if (res_send == nRF_ERR_NO_MODULE){	//module not found
			DEBUG_LOG("RF module not set\r");
			while(1){
				halt_sleep();
			}
		}
		attempt--;
		if (!attempt){
			DEBUG_LOG("attempt end %d\r", period.value);
			attempt = ATTEMPT_SEND_MAX;
			mode = nrf_send_mode;	//reset registraion mode
			allow_press_button();
		}
		sleep_period_set(period);
	}
}