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


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "ds18b20.h"
#include "avr/sleep.h"


#define SLEEP_WDT_S			8										//Интервал сна таймера WDT
#define SLEEP_PERIOD_10MIN	((10*60)/SLEEP_WDT_S)					//Количество срабатываний WDT для интервала сна 10 минут
#define ATTEMPT_SEND_MAX	10										//Максимальное количество попыток передачи

#define	WDT_int_on()		WDTCR |= Bit(WDCE) | Bit(WDTIE)			//Включить прерывания от собаки

//ISR(wdt_reset){
	//Interrupts from the watchdog timer, just a stub so that when you wake up, it does not go to reset
//}

int main(void)
{
	uint8_t Attempt = ATTEMPT_SEND_MAX;
	uint16_t Tempr;
	
PORTB.DIRSET = PIN3_bm;	
PORTB.DIRSET = PIN2_bm;
PORTB.OUTCLR = PIN2_bm;
PORTB.OUTSET = PIN2_bm;
	//struct nRF_Response nRF_Answer;

	//	PRR = (1<<PRTIM0) | (1<<PRADC);									//Выключить таймер и АЦП для экономии электричества
	//	wdt_reset();													//Сброс watchdog
	//	sei();
	
	//	nRF_Init();

	while(1){
		
		Tempr = GetTemperature(Attempt);
PORTB.OUTCLR = PIN2_bm;
PORTB.OUTSET = PIN2_bm;
		if (Tempr != SENSOR_NO){
PORTB.OUTCLR = PIN3_bm;
PORTB.OUTSET = PIN3_bm;
		}
		/*
		if (!nRF_Send(Tempr, &nRF_Answer)){							//Хост не ответил
		Attempt--;												//Минус одна попытка
		if (Attempt==0){										//Попытки передачи исчерпаны, переходим на 10 минутный интервал передачи
		nRF_Answer.Cmd = SLEEP_WDT_S;
		nRF_Answer.Data = SLEEP_PERIOD_10MIN;
		}
		else{													//Попытки еще не исчерпаны попытаемся через 1 секунду
		nRF_Answer.Cmd = ATTINY_13A_1S_SLEEP;
		nRF_Answer.Data = SLEEP_PERIOD_10MIN;
		}
		}
		else
		Attempt = ATTEMPT_SEND_MAX;
		

		set_sleep_mode(SLEEP_MODE_PWR_DOWN);						//Режим глубокого сна - просыпается только от собаки или низкого уровня на INT0
		WDTCR |= Bit(WDCE) | (nRF_Answer.Cmd & ((1<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0)));	//Команда засыпания
		for (u08 CountSleep = nRF_Answer.Data; CountSleep; CountSleep--){	//Спим
		PORTB = 0;
		PORTB = 0xff;											//Подтянуть к питанию для экономии энергии
		WDT_int_on();
		sleep_enable();
		sleep_cpu();
		sleep_disable();										//Проснулись -------------------------
		}
		*/
	}
}