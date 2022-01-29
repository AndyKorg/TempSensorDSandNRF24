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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>
#include <stdint.h>

#include "HAL.h"
#include "nRF24L01P.h"
#include "sleep_rtc.h"

#if SENSOR_TYPE == DEVICE_TYPE_DS18B20
#include "ds18b20.h"
#elif SENSOR_TYPE == DEVICE_TYPE_INTER_TEMPR
#include "InternalTemp.h"
#elif SENSOR_TYPE == DEVICE_TYPE_MH_Z19
#include "MHZ19.h"
#endif

#ifdef CONSOLE_DEBUG
#include "usart.h"
#include <stdio.h>
#endif

#define SLEEP_PERIOD_DEFAULT_S 1

#define SLEEP_PERIOD_LONG_M 4 //sleep period if the receiver does not respond after all transmission attempts, minute
#define ATTEMPT_SEND_MAX 20 //Maximum number of attempts to transfer measurement results or registration mode
#define ATTEMPT_READ_SENSOR 10

#define allow_press_button()                             \
do {                                                 \
	BUTTON_PORT.INTFLAGS |= BUTTON_PIN;              \
	BUTTON_INT = BUTTON_INT_TYPE | PORT_PULLUPEN_bm; \
} while (0) //allow button press test

#define WAKE_UP_OFF 0
#define WAKE_UP_ON 1
#define halt_sleep(int_allow)        \
do {                             \
	period.dim = dd_Min;         \
	period.value = UINT16_MAX;   \
	if (int_allow == WAKE_UP_ON) \
	allow_press_button();    \
	sleep_period_set(period);    \
} while (0)

volatile static uint8_t mode = nrf_send_mode;

ISR(BUTTON_INT_VECT)
{
	if (BUTTON_PORT.INTFLAGS & BUTTON_PIN) { //test button pressed
		if (!(BUTTON_PORT.IN & BUTTON_PIN)) {
			mode = nrf_reg_mode;
			BUTTON_INT = PORT_PULLUPEN_bm; //interrupt off
			sleep_period_stop();
		}
		//BUTTON_PORT.INTFLAGS |= BUTTON_PIN;		//allowing retry only after attempts have been exhausted. see below.
	}
}

#ifdef CONSOLE_DEBUG
bool console_cmd(cmd_t cmd)
{
	return true;
}
#endif

inline void pin_off_unused(void)
{
	//PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	//PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;	//MOSI
	//PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;	//MISO
	//PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;	//SCK
	//PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;	//CSN
	//PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;	//CE
	PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc; //IRQ
	PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;

	PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc; //SDA
	PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc; //SCL
	#if SENSOR_TYPE != DEVICE_TYPE_MH_Z19s
	PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc; //TXD

	PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc; //RXD
	#endif
	#if SENSOR_TYPE != DEVICE_TYPE_DS18B20
	PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc; //1-wire
	#endif
	PORTB.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;

	//PORTC.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;	//button

	#if SENSOR_TYPE != DEVICE_TYPE_MH_Z19s
	PORTC.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc; //power mh-z19
	#endif
	PORTC.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTC.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc; //no pin in MK
	PORTC.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc; //no pin in MK
}

int main(void)
{
	//	#include <avr/xmega.h>
	//	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm);

PORTB.DIRSET = PIN4_bm;
	uint8_t attempt = ATTEMPT_SEND_MAX;
	uint16_t SensorValue;

	#ifdef CONSOLE_DEBUG
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

	while (1) {

		DEBUG_LOG("main loop go\r");

		if (mode == nrf_send_mode) { //send data to PRX
			uint8_t buf[4];
			#if SENSOR_TYPE == DEVICE_TYPE_DS18B20
			SensorValue = GetTemperDS18b20(ATTEMPT_READ_SENSOR);
			#elif SENSOR_TYPE == DEVICE_TYPE_INTER_TEMPR
			SensorValue = GetTemperature(ATTEMPT_READ_SENSOR);
			#elif SENSOR_TYPE == DEVICE_TYPE_MH_Z19
			SensorValue = 0;
			if (MHZ19_ready(ATTEMPT_READ_SENSOR)){
				SensorValue = GetCO2_MHZ19(ATTEMPT_READ_SENSOR);
			}
			#endif
			buf[0] = SENSOR_TYPE;
			buf[1] = 0;
			buf[2] = (uint8_t)(SensorValue >> 8);
			buf[3] = (uint8_t)SensorValue;
			DEBUG_LOG("t= %04x\r", SensorValue);
			nrf_err_t res_send = nRF_SendData(buf, 4, &nRF_Answer);
			switch (res_send) {
				case nRF_OK:
				period.dim = dd_Min;
				period.value = SLEEP_PERIOD_LONG_M; //white long delay if answer not correct TODO:Добавить проверку типа команды и датчика
				if (nRF_Answer.Len == DEVICE_ANSWER_LEN) { //response in the measurement transfer mode, we check the correctness of the response and set the sleep period depending on the response
					period.value = *((uint16_t*)(nRF_Answer.Data + DEVICE_ANSWER_PERIOD_OFFSET));
					attempt = ATTEMPT_SEND_MAX;
				}
				DEBUG_LOG("send OK sleep %d\r", period.value);
				allow_press_button();
				#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
				if ((period.dim = dd_Min) && (period.value < 30)){	//if the sleep period is too long, the CO2 sensor will turn off
					MHZ19_stop();	
				}
				#endif
				sleep_period_set(period);
				break;
				case nRF_ERR_NO_ANSWER: //PRX not answer, we will check there are attempts, if there are we will try, if not then we sleep.
				{
					period_t tmp;
					tmp.dim = dd_Sec;
					tmp.value = SLEEP_PERIOD_DEFAULT_S;
					if (!attempt) {
						tmp.value = SLEEP_PERIOD_LONG_M; //white long delay if answer not correct TODO:Добавить проверку типа команды и датчика
						tmp.dim = dd_Min;
						attempt = ATTEMPT_SEND_MAX;
					}
					attempt--;
					DEBUG_LOG("PRX not answer next attempt %d\r", attempt);
					allow_press_button();
					sleep_period_set(tmp);
					break;
				}
				case nRF_ERR_NO_MODULE: //no RF module, don't wake up
				DEBUG_LOG("RF module not responce! halt\r");
				while (1){
					#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
					MHZ19_stop();
					#endif
					halt_sleep(WAKE_UP_OFF);
				}
				break;
				case nRF_ERR_ADDR_NOT_FOUND: //receiver address not set. fall asleep before pressing the button.
				DEBUG_LOG("address not set! halt\r");
				while (mode == nrf_send_mode){
					#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
					MHZ19_stop();
					#endif
					halt_sleep(WAKE_UP_ON);
				}
				break;
				default:
				break;
			}
			continue;
		}
		//registration mode
		if (nRF_TransmitReg(&nRF_Answer) == nRF_OK){
			period_t delay;
			delay.dim = dd_mSec;
			delay.value = 200;
			sleep_period_set(delay);
		}
		mode = nrf_send_mode;
	}
}
