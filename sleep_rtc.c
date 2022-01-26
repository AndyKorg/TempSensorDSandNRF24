/*
\brief sleep for a certain period.
Always includes interrupts!
*/

#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include "sleep_rtc.h"

#ifdef CONSOLE_DEBUG
#include <stdio.h>
#include "usart.h"
#endif
#if SENSOR_TYPE == DEVICE_TYPE_MH_Z19
#include "MHZ19.h"
#endif

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

typedef struct{
	sleep_period_t period;
	uint32_t milliseconds;
} period_fix_t;

static const period_fix_t period_fix_ms[] = {
	{slp4MS, (4UL*1000)/RTC_F},		//0
	{slp8MS, (8UL*1000)/RTC_F},		//1
	{slp16MS, (16UL*1000)/RTC_F},	//2
	{slp32MS, (32UL*1000)/RTC_F},	//3
	{slp62MS, (64UL*1000UL)/RTC_F},	//4
	{slp125MS, (128UL*1000)/RTC_F},	//5
	{slp250MS, (256UL*1000)/RTC_F},	//6
	{slp05S, (512UL*1000)/RTC_F},	//7
	{slp1S, (1024UL*1000)/RTC_F},	//8
	{slp2S, (2048UL*1000)/RTC_F},	//9, SLPEEP_MAX_STBY_FIX_IDX see code below
	{slp4S, (4096UL*1000)/RTC_F},	//10
	{slp8S, (8192UL*1000)/RTC_F},	//11
	{slp16S, (16384UL*1000)/RTC_F},	//12
	{slp32S, (32768UL*1000)/RTC_F},	//13
};

#define SLPEEP_MAX_STBY_FIX_IDX		9	//maximum sleep period in stand-by mode, index period_fix_ms array
//the sleep period above this value is observed inaccurately and in the power-down mode

static volatile bool sleep_stop;	//variable setting in interrupt!
static volatile timeout_cb_t timeout_func = NULL;

void sleep_period_stop(void){
	sleep_stop = true;
}

ISR(RTC_PIT_vect){
	SLEEP_TIMER.PITINTFLAGS = RTC_PI_bm;
}

ISR(RTC_CNT_vect){
	if (timeout_func){
		timeout_func();
		timeout_func = NULL;
		SLEEP_TIMER.CTRLA = 0;//stop RTC
		return;
	}
	SLEEP_TIMER.INTFLAGS = RTC_CMP_bm | RTC_OVF_bm;
}

static void internal_pwrdwn_sleep(sleep_period_t period_num){
	if (!period_num) period_num = slp1S;
	if (period_num >= slpMAX){
		period_num = slp32S;
	}
	while((RTC.STATUS & RTC_CTRLBUSY_bm) || (SLEEP_TIMER.PITSTATUS & RTC_CTRLBUSY_bm));	//Wait for all register to be synchronized
	SLEEP_TIMER.PITCTRLA = ((period_num)<<3) | RTC_PITEN_bm;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);	//Deep sleep mode - wakes up only from RTC
	bool i_state = isr_state();
	sei();
	DEBUG_CON_FREE();
	SLEEP_TIMER.PITINTCTRL = RTC_PI_bm;		//interrupt	on
	debugUp();
	sleep_enable();
	sleep_cpu();
	sleep_disable();						//woke up -------------------------
	debugDown();
	SLEEP_TIMER.PITINTCTRL &= ~RTC_PI_bm;	//interrupt off
	if (!i_state){
		cli();
	}
}

static void internal_stdby_sleep(uint16_t period_ms){
	while((RTC.STATUS & RTC_CTRLBUSY_bm) || (SLEEP_TIMER.PITSTATUS & RTC_CTRLBUSY_bm));	//Wait for all register to be synchronized
	SLEEP_TIMER.CTRLA = RTC_RUNSTDBY_bm | RTC_RTCEN_bm;
	SLEEP_TIMER.CNT = 0;
	SLEEP_TIMER.PER = period_ms;
	SLEEP_TIMER.INTCTRL = RTC_OVF_bm;		//interrupt on
	set_sleep_mode(SLEEP_MODE_STANDBY);
	bool i_state = isr_state();
	sei();
	DEBUG_CON_FREE();
	debugUp();
	sleep_enable();
	sleep_cpu();
	sleep_disable();						//woke up -------------------------
	debugDown();
	if (!i_state){
		cli();
	}
	SLEEP_TIMER.INTCTRL &= ~RTC_OVF_bm;		//interrupt off
	if (sleep_stop){
		return;
	}
}


void sleep_period_set(period_t period){
	uint32_t period_ms;
	sleep_stop = false;
	//calculate period in meilliseconds
	switch (period.dim){
		case dd_mSec:
		period_ms = (uint32_t) period.value;
		break;
		case dd_Sec:
		period_ms = (uint32_t) period.value * 1000;	//seconds per minute, and milliseconds per second
		break;
		case dd_Min:
		period_ms = (uint32_t) period.value * 60 * 1000;	//seconds per minute, and milliseconds per second
		break;
		default:
		return;
	}
	//white usart transsmit
	while (usart_is_busy());
//	return; ////////////////////// ---------------------------
	//delay period
	while(period_ms){
		if (period_ms < period_fix_ms[SLPEEP_MAX_STBY_FIX_IDX].milliseconds){
			internal_stdby_sleep(period_ms);
			return;
		}
		uint8_t idx = 0;
		idx = sizeof(period_fix_ms)/sizeof(period_fix_t);
		if (period_ms < period_fix_ms[(sizeof(period_fix_ms)/sizeof(period_fix_t))-1].milliseconds){
			for(idx=SLPEEP_MAX_STBY_FIX_IDX; idx < sizeof(period_fix_ms)/sizeof(period_fix_t); idx++){
				if (period_ms < period_fix_ms[idx].milliseconds){
					break;
				}
			}
		}
		idx--;
		uint32_t count_sleep = period_ms/period_fix_ms[idx].milliseconds;
		period_ms = period_ms % period_fix_ms[idx].milliseconds;
		while(count_sleep){
			internal_pwrdwn_sleep(period_fix_ms[idx].period);
			if (sleep_stop){
				return;
			}
			count_sleep--;
		}
	}
}

void timeout_start(uint16_t sec, timeout_cb_t func_cb){
	if (timeout_func) return;
	if (!func_cb) return;
	timeout_func = func_cb;
	while((RTC.STATUS & RTC_CTRLBUSY_bm) || (SLEEP_TIMER.PITSTATUS & RTC_CTRLBUSY_bm));	//Wait for all register to be synchronized
	SLEEP_TIMER.CTRLA = RTC_PRESCALER_DIV1024_gc | RTC_RTCEN_bm;
	SLEEP_TIMER.CNT = 0;
	SLEEP_TIMER.PER = sec;
	SLEEP_TIMER.INTCTRL = RTC_OVF_bm;		//interrupt on
}

void timeout_stop(void){
	if (!timeout_func) return;
	timeout_func = NULL;
	SLEEP_TIMER.CTRLA = 0;//stop RTC
}

void timeout_reset(void){
	if (!timeout_func) return;
	SLEEP_TIMER.CNT = 0;//reset RTC
}