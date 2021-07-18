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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "sleep_rtc.h"
#include "ds18b20.h"
#include "nRF24L01P.h"


#define SLEEP_PERIOD_DEFAULT	slp1S
//#define SLEEP_PERIOD_DEFAULT	slp32S

//#define SLEEP_PERIOD_LONG		slp32S
#define SLEEP_PERIOD_LONG		slp4S
#define ATTEMPT_SEND_MAX		10		//Maximum number of attempts to transfer measurement results or registration mode, over 10 second
#define ATTEMPT_REG_MAX			10
#define ATTEMPT_READ_SENSOR		10

volatile static uint8_t mode = nrf_send_mode;

ISR(BUTTON_INT_VECT){
	if (BUTTON_PORT.INTFLAGS & BUTTON_PIN){
		if (!(BUTTON_PORT.IN & BUTTON_PIN)){
			mode = nrf_reg_mode;
		}
		BUTTON_PORT.INTFLAGS |= BUTTON_PIN;
	}
}

int main(void)
{
	//	#include <avr/xmega.h>
	//	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm);

	uint8_t attempt = ATTEMPT_SEND_MAX;
	uint16_t Tempr;
	
	PORTB.DIRSET = PIN2_bm;
	PORTB.OUTCLR = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;
	nrf_Response_t nRF_Answer;

	BUTTON_PORT.DIRCLR = BUTTON_PIN;
	BUTTON_INT = BUTTON_INT_TYPE | PORT_PULLUPEN_bm;

	RTC_1KHZ_init();
	//	PRR = (1<<PRTIM0) | (1<<PRADC);									//Выключить таймер и АЦП для экономии электричества
	//	wdt_reset();													//Сброс watchdog
	mode = nrf_send_mode;
	sei();
	
	nRF_Init();

	sleep_period_t period = SLEEP_PERIOD_DEFAULT;		//Attempts have not yet been exhausted, we will try in 1 second
	while(1){
		
		Tempr = GetTemperature(ATTEMPT_READ_SENSOR);
		uint8_t buf[4];
		buf[0] = DEVICE_TYPE_MH_Z19;
		buf[1] = DEVICE_TYPE_MH_Z19;
		buf[2] = (uint8_t)(Tempr>>8);
		buf[3] = (uint8_t)Tempr;
		nrf_err_t res_send = nRF_Send(mode, buf, 4, &nRF_Answer);
		if (res_send == nRF_OK){
			if (mode == nrf_send_mode){	//data transferred
				period = SLEEP_PERIOD_LONG;	//white long delay if answer not correct TODO:Добавить проверку типа команды и датчика
				if (nRF_Answer.Len == DEVICE_ANSWER_LEN){	//response in the measurement transfer mode, we check the correctness of the response and set the sleep period depending on the response
					period = *(nRF_Answer.Data+DEVICE_ANSWER_NUM_PERIOD);
					attempt = ATTEMPT_SEND_MAX;
				}
				sleep_period_set(period);
				continue;
			}
			//registration mode
			if (nRF_real_address_is_set()){
				//registration completed successfully
				attempt = ATTEMPT_SEND_MAX;
				mode = nrf_send_mode;	//send data
				continue;	//!!!!!!!!!!!!!!!!!!!!! continue without sleep!
			}
		}
		if (res_send == nRF_ERR_NO_MODULE){	//module not found
			while(1){
				sleep_period_set(slp32S);	//halt-sleep
			}
		}
		attempt--;
		if (!attempt){
			sleep_period_set(SLEEP_PERIOD_LONG);
			attempt = ATTEMPT_SEND_MAX;
			mode = nrf_send_mode;	//reset registraion mode
		}
	}
}