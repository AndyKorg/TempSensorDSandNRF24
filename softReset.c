#include "HAL.h"
#include <util/delay.h>
#include "softReset.h"


void soft_reset_init(void){
	//software reset
	RESET_SOFT_PORT.DIRCLR = RESET_SOFT_PIN;
	RESET_SOFT_PORT.PIN0CTRL |= PORT_PULLUPEN_bm;
	RESET_SOFT_PORT.OUTSET = RESET_SOFT_PIN;
}

void soft_reset_check(void){
	if (!(RESET_SOFT_PORT.IN & RESET_SOFT_PIN)) { //software reset
		_delay_ms(100);
		cli();
		while(!(RESET_SOFT_PORT.IN & RESET_SOFT_PIN));
		_PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRE_bm);//reset!
	}
}
