/*
\brief sleep for a certain period.
Always includes interrupts!
 */ 


#ifndef SLEEP_RTC_H_
#define SLEEP_RTC_H_

#include <avr/io.h>
#include "HAL.h"

#define RTC_1KHZ_init()			do {while(SLEEP_TIMER.STATUS > 0);SLEEP_TIMER.CLKSEL = RTC_CLKSEL_INT1K_gc;} while (0)

typedef enum{			//see RTC_PERIOD_t
	slp4MS = 0x1,		// CYC4 4 cycles
	slp8MS = 0x2,		// CYC8 8 cycles
	slp16MS = 0x3,		// CYC16 16 cycles
	slp32MS = 0x4,		// CYC32 32 cycles
	slp62MS = 0x5,		// CYC64 64 cycles
	slp125MS = 0x6,		// CYC128 128 cycles
	slp250MS = 0x7,		// CYC256 256 cycles
	slp05S = 0x8,		// CYC512 512 cycles
	slp1S = 0x9,		// CYC1024 1024 cycles
	slp2S = 0xA,		// CYC2048 2048 cycles
	slp4S = 0xB,		// CYC4096 4096 cycles
	slp8S = 0xC,		// CYC8192 8192 cycles
	slp16S = 0xD,		// CYC16384 16384 cycles
	slp32S = 0xE,		// CYC32768 32768 cycles
	slpMAX,
} sleep_period_t;

void sleep_init(void);
/*
\brief Chip falls asleep in! although interrupts are allowed!
*/
void sleep_period_set(sleep_period_t period);

#endif /* SLEEP_RTC_H_ */