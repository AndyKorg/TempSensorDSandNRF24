/*
 * Работа с модулем nRF24L01P через soft-spi
 *
 */ 


#ifndef NRF24L01P_H_
#define NRF24L01P_H_

#ifdef DEBUG
#define LedOn() PORTA  &= ~(1<<PORTA0)
#define LedOff() PORTA |= (1<<PORTA0)
#endif


#define nRF_PIPE				4				//Номер используемого канала
#define nRF_SEND_LEN			4				//Длина передаваемого пакета данных. Должна совпадать со значением nRF_SEND_LEN в файле конфигурации часов ....\ClockMatrix\nRF24L01P.h
#define nRF_TMPR_ATTNY13_SENSOR	0x01			//Датчик температуры на Attiny13. Должна совпадать со значением nRF_TMPR_ATTNY13_SENSOR в файле конфигурации часов ....\ClockMatrix\nRF24L01P.h
#define nRF_ACK_LEN				3				//Длина ответа от часов при приеме пакета от радиодатчика. Должна совпадать со значением nRF_ACK_LEN в файле конфигурации часов ....\ClockMatrix\nRF24L01P.h
#define nRF_RESERVED_BYTE		0				//Байт зарезервированный для будущего использования

#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "nRF24L01P Reg.h"
#include "bits_macros.h"

struct nRF_Response{							//Ответ от передатчика
	u08	Cmd;									//Команда
	u16 Data;									//Параметр команды
	};
	
#define nRF_NO_RECIV			0				//Ничего не принято результате обмена с хостом.
#define nRF_REPEAT_MAX			7				//Количество попыток повтора передачи при неудаче. 0 - отключить автоповтор
#define nRF_REPEAT_INTERVAL		nRF_RETR_500US	//Интервал автоповтора
/*
#define nRF_MAX_TIMEOUT			((4UL*16UL)*2UL)//Максимальная длительность операции передачи пакета в мс. 4000 мкс = 4 мс - максимальный период перед повтором пакета по даташиту, 16 - максимальное количество повторов. Увеличено в 2 раза для надежности
#define TIMER_DEVIDER			1024UL
#define TIMER_INTERVAL			(F_CPU/TIMER_DEVIDER/1000)	//Значение для регистра сравнения для получения периода 1 мс

#if (TIMER_DEVIDER == 1024UL)
	#define TimerStart()		do {TCCR0A = (1<<WGM02) | (1<<WGM01); \
									TCCR0B = (0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);\
									TCNT0 = 0;} while(0)	//Включить таймер
#else
	#error "TIMER_DEVIDER not valid"
#endif

#define TimerStop()				TCCR0B = (0<<CS02) | (0<<CS01) | (0<<CS00);	//Выключить таймер
*/

#if ((nRF_REPEAT_MAX<0) && (nRF_REPEAT_MAX>15))
#error "Incorrect value nRF_REPEAT_MAX. The nRF_REPEAT_MAX should be between 1 to 32"
#endif

#if ((nRF_ACK_LEN<1) && (nRF_ACK_LEN>32))
	#error "Incorrect value nRF_ACK_LEN. The nRF_ACK_LEN should be between 1 to 32"
#endif

#if (nRF_PIPE == 0)
	#define nRF_PRE_ADR_PIPE	0xe7
	#define nRF_ADR_PIPE		0xe7
#elif (nRF_PIPE == 1)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc2
#elif (nRF_PIPE == 2)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc3
#elif (nRF_PIPE == 3)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc4
#elif (nRF_PIPE == 4)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc5
#elif (nRF_PIPE == 5)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc6
#else
	#error "Bad pipe! Pipe num from 0 to 5"
#endif

#if ((nRF_ACK_LEN<1) && (nRF_ACK_LEN>32))
	#error "Incorrect value nRF_ACK_LEN. The nRF_ACK_LEN should be between 1 to 32"
#endif

#define nRF_DDR					DDRB			//Программный SPI
#define nRF_PORTIN				PINB
#define nRF_PORT				PORTB
#define nRF_SPI_MOSI			PINB1
#define nRF_SPI_MISO			PINB3
#define nRF_SPI_SCK				PINB4
#define nRF_CSN					PINB0			//Выбор кристалла
#define nRF_CE					PINB2			//Запуск операции

#define nRF_SELECT()			ClearBit(nRF_PORT, nRF_CSN)
#define nRF_DESELECT()			SetBit(nRF_PORT, nRF_CSN)

#define nRF_GO()				SetBit(nRF_PORT, nRF_CE)
#define nRF_STOP()				ClearBit(nRF_PORT, nRF_CE)

//Настройка прескаллера WDT для ATTINY13A
#define ATTINY_13A_WDP0 0
#define ATTINY_13A_WDP1 1
#define ATTINY_13A_WDP2 2
#define ATTINY_13A_WDP3 5
#define ATTINY_13A_16MS_SLEEP	((0<<ATTINY_13A_WDP3) | (0<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	//срабатывание собаки через 16 мс
#define ATTINY_13A_05S_SLEEP	((0<<ATTINY_13A_WDP3) | (1<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	// -/- 0,5 секунд
#define ATTINY_13A_1S_SLEEP		((0<<ATTINY_13A_WDP3) | (1<<ATTINY_13A_WDP2) | (1<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	// -/- 1 секунду
#define ATTINY_13A_8S_SLEEP		((1<<ATTINY_13A_WDP3) | (0<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (1<<ATTINY_13A_WDP0))	// -/- 8 секунд

void nRF_Init(void);
u08 nRF_Send(u16 Data, struct nRF_Response *nRF_Resp);		//Передача данных Data и прием ответа nRF_Resp

#endif /* NRF24L01P_H_ */