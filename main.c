/*
 * external sensors - temperature or mht-z19
 * data transmission over the radio channel to nRF24L01+
 * the temperature sensor is battery powered, so the temperature transfer over a long period.
 * The air sensor is powered by the pump, therefore, the requirements for the consumption mode are simplified.
 */ 

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/xmega.h>

#include "HAL.h"
#include "one_wire.h"

static uint8_t device_present = 0;

bool sensor_answer(uint8_t *value){
	PORTB.OUTCLR = PIN3_bm;
	PORTB.OUTSET = PIN3_bm;
	if (*value){
		PORTB.OUTCLR = PIN2_bm;
		device_present = 1;
	}
	return true;
}

bool sensor_send(uint8_t *value){
	return true;
}

int main(void)
{
	//wdt_enable(WDT_PERIOD_1KCLK_gc);

	PORTB.DIRSET = PIN3_bm;
	PORTB.DIRSET = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;

	PORTB.OUTCLR = PIN3_bm;
	PORTB.OUTSET = PIN3_bm;
	
	OneWareIni();
	sei();
    OneWareReset(sensor_answer);
    while (1){
		if (device_present){
			OneWareSendByte(0x33, sensor_send);
		}
    }
}

