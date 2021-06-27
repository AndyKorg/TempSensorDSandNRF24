/*
* HAL schematic
*/
#ifndef HAL_H_
#define HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CLOCK		20000000UL
#define F_CPU		(F_CLOCK/6)	//default prescaller

#ifdef DEBUG

//software reset
#define RESET_SOFT_PORT			PORTC
#define RESET_SOFT_PIN			PIN0_bm
//start impuls
#define START_PORT				PORTC
#define START_PIN				PIN4_bm

#define DEBUG_PORT				PORTA
#define DEBUG_PIN				PIN7_bm
#endif

//------------------ ONE WIRE PORT -----------------------------------
#define ONE_WIRE_PORT			PORTB
#define ONE_WIRE_PIN			PIN4_bm
#define ONE_WIRE_PIN_CTRL		PIN4CTRL
#define ONE_WIRE_PORT_INT		PORTB_PORT_vect

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
//polled
#define nRF_SPI					SPI0
#define nRF_PORT				PORTA
#define nRF_SPI_MOSI			PIN1_bm
#define nRF_SPI_MISO			PIN2_bm
#define nRF_SPI_SCK				PIN3_bm
#define nRF_CSN					PIN4_bm			//cristall select
#define nRF_CE					PIN5_bm			//start operation
#endif
#endif

//------------------ TIMER -----------------------------------

#endif /* HAL_H_ */