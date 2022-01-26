/*
\brief sleep for a certain period.
Always includes interrupts!
 */ 


#ifndef SLEEP_RTC_H_
#define SLEEP_RTC_H_

#include <avr/io.h>
#include "HAL.h"

#define RTC_F	1024UL
#if (RTC_F == 1024)
#define RTC_1KHZ_init()			do {while(SLEEP_TIMER.STATUS > 0);SLEEP_TIMER.CLKSEL = RTC_CLKSEL_INT1K_gc;} while (0)
#endif

typedef enum{
	dd_mSec,			//dimension of delay
	dd_Sec,
	dd_Min,
} dimension_t;

typedef struct{
	uint16_t value;
	dimension_t dim;
} period_t;

/*
\brief Chip falls asleep in! although interrupts are allowed!
*/
void sleep_period_set(period_t period);
/*
\brief stop sleep period
*/
void sleep_period_stop(void);

typedef void (*timeout_cb_t)(void);
/*
\brief start timeout timer in second
func_cb - callback function. Called when the timeout expires. if not set, then the timeout is not triggered.
Recall does not start any action until either the countdown is stopped or the timeout expires.
*/
void timeout_start(uint16_t sec, timeout_cb_t func_cb);
/*
\brief stop timeout timer
Callback function not called!
if the timer has not started, then no change is made.
*/
void timeout_stop(void);

/*
\brief reset timeout timer
Callback function not called!
if the timer has not started, then no change is made.
*/
void timeout_reset(void);

#endif /* SLEEP_RTC_H_ */