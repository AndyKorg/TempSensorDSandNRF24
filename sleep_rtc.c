/*
\brief sleep for a certain period.
Always includes interrupts!
*/

#include <avr/sleep.h>
#include <stdbool.h>
#include "sleep_rtc.h"

ISR(RTC_PIT_vect){
	SLEEP_TIMER.PITINTFLAGS = RTC_PI_bm;
}

void sleep_period_set(sleep_period_t period){
	while(SLEEP_TIMER.PITSTATUS > 0);		//Wait for all register to be synchronized
	if (!period) period = slp1S;
	if (period >= slpMAX){
		period = slp32S;
	}
	SLEEP_TIMER.PITCTRLA = ((period)<<3) | RTC_PITEN_bm;
	SLEEP_TIMER.PITINTCTRL = RTC_PI_bm;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);						//Deep sleep mode - wakes up only from RTC
	bool i_state = isr_state();
	sei();
	sleep_enable();
	PORTB.OUTCLR = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;
	sleep_cpu();
	sleep_disable();										//woke up -------------------------
	PORTB.OUTCLR = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;
	PORTB.OUTCLR = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;
	if (!i_state){
		cli();
	}
}
