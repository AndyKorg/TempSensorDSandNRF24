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

#define main_bus_set()	do {PORTMUX.CTRLB &= ~PORTMUX_USART0_bm;} while (0);
#define alt_bus_set()	do {PORTMUX.CTRLB |= PORTMUX_USART0_bm;} while (0);

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

//------------------ TIMER -----------------------------------

#endif /* HAL_H_ */