/*
* HAL schematic
*/
#ifndef HAL_H_
#define HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "external_device.h"

#define F_CLOCK		20000000UL
#undef F_CPU
#define F_CPU		(F_CLOCK/6UL)	//default prescaller
#define isr_state()	(SREG & CPU_I_bm)

//------------------ select type sensor  -----------------------------------
//#define SENSOR_TYPE		DEVICE_TYPE_DS18B20
//#define SENSOR_TYPE		DEVICE_TYPE_INTER_TEMPR		//temperature sensor into chip
#define SENSOR_TYPE		DEVICE_TYPE_MH_Z19

#define CONSOLE_DEBUG
#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
#undef CONSOLE_DEBUG
#endif


#ifdef CONSOLE_DEBUG
#define DEBUG_PORT	PORTC
#define DEBUG_PIN	PIN5_bm

#define debugPortIni()	do {DEBUG_PORT.DIRSET = DEBUG_PIN;} while (0)
#define debugBoth()		do {DEBUG_PORT.OUTTGL = DEBUG_PIN; DEBUG_PORT.OUTTGL = DEBUG_PIN;} while (0)
#define debugUp()		do {DEBUG_PORT.OUTSET = DEBUG_PIN;} while (0)
#define debugDown()		do {DEBUG_PORT.OUTCLR = DEBUG_PIN;} while (0)

#define DEBUG_CONSOLE_PORT	PORTB
#define DEBUG_CONSOLE_PIN	PIN2_bm

#define debugConPortInit()	do {DEBUG_CONSOLE_PORT.DIRSET = DEBUG_CONSOLE_PIN;} while (0)
	
#define DEBUG_CON_FREE()	do {while(usart_is_busy());} while(0)

#define DEBUG_LOG(format, ...)	do {printf(format, ##__VA_ARGS__);} while (0)

#else

#define debugPortIni()	
#define debugBoth()		
#define debugUp()		
#define debugDown()		

#define debugConPortInit()
#define DEBUG_LOG(format, ...)
#define DEBUG_CON_FREE()

#endif

//------------------ SEND (TEST) BUTTON -----------------------------------
#define BUTTON_PORT				PORTC
#define BUTTON_PIN				PIN0_bm
#define BUTTON_INT				BUTTON_PORT.PIN0CTRL
#define BUTTON_INT_TYPE			PORT_ISC_LEVEL_gc
#define BUTTON_INT_VECT			PORTC_PORT_vect

//------------------ ONE WIRE PORT IF SENSOR_TYPE = DEVICE_TYPE_DS18B20 ---
#define ONE_WIRE_PORT			PORTB
#define ONE_WIRE_PIN			PIN4_bm
#define ONE_WIRE_PIN_CTRL		PIN4CTRL
#define ONE_WIRE_PORT_INT		PORTB_PORT_vect

//------------------ MH-Z19 -----------------------------------
#define MHZ19_PORT_USART		PORTB
#define MHZ19_PORT_TXD_PIN		PIN2_bm
#define MHZ19_PORT_RXD_PIN		PIN3_bm
#define MHZ19_PORT_POWER		PORTC
#define MHZ19_PIN_POWER			PIN1_bm
#define mhz19_power_off()		do {MHZ19_PORT_POWER.OUTSET = MHZ19_PIN_POWER;} while (0)
#define mhz19_power_on()		do {MHZ19_PORT_POWER.OUTCLR = MHZ19_PIN_POWER;} while (0)
#define mhz19_PortInit()		do {\
	MHZ19_PORT_USART.DIRSET = MHZ19_PORT_TXD_PIN; \
	MHZ19_PORT_USART.DIRCLR = MHZ19_PORT_RXD_PIN;\
	MHZ19_PORT_USART.PIN3CTRL = PORT_PULLUPEN_bm;\
	MHZ19_PORT_POWER.DIRSET = MHZ19_PIN_POWER;\
	mhz19_power_on();\
	} while (0)

//------------------ nRF24L01+  -----------------------------------
//this not tested! #define nRF_SPI_SOFT							//comment out if using hard SPI

#ifdef nRF_SPI_SOFT
#if __AVR_ARCH__ >= 100
#define nRF_DDR					PORTB.DIR
#define nRF_PORTIN				PORTB
#define nRF_PORT				PORTB
#define nRF_SPI_MOSI			PIN1_bm
#define nRF_SPI_MISO			PIN3_bm
#define nRF_SPI_SCK				PIN4_bm
#define nRF_CSN					PIN0_bm			//cristall select
#define nRF_CE					PIN2_bm			//start operation
#else
#define nRF_DDR					DDRB
#define nRF_PORTIN				PINB
#define nRF_PORT				PORTB
#define nRF_SPI_MOSI			PINB1
#define nRF_SPI_MISO			PINB3
#define nRF_SPI_SCK				PINB4
#define nRF_CSN					PINB0			//cristall select
#define nRF_CE					PINB2			//start operation
#endif
#else
#if __AVR_ARCH__ >= 100
//polled mode
#define nRF_SPI					SPI0
#define nRF_PORT				PORTA
#define nRF_SPI_MOSI			PIN1_bm
#define nRF_SPI_MISO			PIN2_bm
#define nRF_SPI_MISO_CRL		PIN2CTRL
#define nRF_SPI_SCK				PIN3_bm
#define nRF_CSN					PIN4_bm			//cristall select
#define nRF_CE					PIN5_bm			//start operation
#endif
#endif

#define nRF_EEPROM				0				//offset for address and address registration status

//------------------ TIMER -----------------------------------
#define SLEEP_TIMER				RTC				//sleep and timeout timer

#endif /* HAL_H_ */