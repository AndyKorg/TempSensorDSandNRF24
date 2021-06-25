/*
* HAL schematic
*/
#ifndef HAL_H_
#define HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU		20000000UL
#define F_MAIN		(F_CPU/6)
#define DELAY_US(x)	(F_MAIN/(1000000/x))

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
//15 us 1-wire bus----------------------------
#define ONE_WIRE_TIMER			TCA0.SINGLE
#define ONE_WIRE_INT			TCA0_OVF_vect
#define one_wire_int_reset()	do {ONE_WIRE_TIMER.INTFLAGS = TCA_SINGLE_OVF_bm;} while (0);
#define one_wire_timer_init()	do {\
	ONE_WIRE_TIMER.CNT = 0;\
	ONE_WIRE_TIMER.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc | TCA_SINGLE_ALUPD_bm;\
	ONE_WIRE_TIMER.INTCTRL = TCA_SINGLE_OVF_bm;\
} while (0)
#define one_wire_timer_start(x)	do {\
	ONE_WIRE_TIMER.PER = x;\
	ONE_WIRE_TIMER.CNT = 0;\
	ONE_WIRE_TIMER.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;\
} while (0)
#define one_wire_timer_stop() do {	ONE_WIRE_TIMER.CTRLA &= ~TCA_SINGLE_ENABLE_bm;\
} while (0)

#endif /* HAL_H_ */