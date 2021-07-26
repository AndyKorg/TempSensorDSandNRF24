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

#endif /* SLEEP_RTC_H_ */