#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "HAL.h"
#ifdef DEBUG
#include <util/delay.h>
#include "softReset.h"
#endif

#include "mh_z19.h"
#include "FIFO.h"

#define BAUD_RATE		9600

#define UROUND(x) ((2UL*(x)+1)/2)

#define UART_BUF_LEN	128

uint8_t const mh_z19_start[] = {0XFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79},
mh_z19_result[] = {0XFF, 0x86};

#define RES_PILOT_LEN	(sizeof(mh_z19_result)/sizeof(mh_z19_result[0]))
#define RES_LEN			9	//result len packet

volatile FIFO(UART_BUF_LEN) txBuf, rxBuf;

ISR(USART0_DRE_vect){
	if (!FIFO_IS_EMPTY(txBuf)){
		USART0.TXDATAL = FIFO_FRONT(txBuf);
		FIFO_POP(txBuf);
		return;
	}
}

ISR(USART0_RXC_vect){
	
	uint8_t data = USART0.RXDATAL;
	static uint8_t pilot_num = 0, result_num = 0, res_h_byte, res_l_byte;

	if (pilot_num < RES_PILOT_LEN){
		if (data != mh_z19_result[pilot_num]){
			pilot_num = 0;
			return;
		}
	}
	else if (pilot_num == RES_PILOT_LEN){
		result_num = 1;
		res_h_byte = data;
	}
	else if (pilot_num > RES_PILOT_LEN){
		if (pilot_num == (RES_LEN-1)){ //end packet
			pilot_num = 0;
			if (FIFO_SPACE(rxBuf) >= 2){
				FIFO_PUSH(rxBuf, res_h_byte);
				FIFO_PUSH(rxBuf, res_l_byte);
			}
			return;
		}
		if (result_num == 1){
			res_l_byte = data;
		}
		result_num++;
	}
	pilot_num++;
}

uint16_t mh_19_read(void){

	main_bus_set();
	
	PORTB.DIRSET = PIN2_bm;
	USART0.BAUD = UROUND(64UL*F_CPU/32/BAUD_RATE);
	
	FIFO_FLUSH(txBuf);
	FIFO_FLUSH(rxBuf);

	USART0.CTRLB |= USART_TXEN_bm | USART_RXEN_bm;
	USART0.CTRLA |= USART_DREIF_bm | USART_RXCIE_bm;
	
	//send start measure
	uint8_t byte;
	for (byte = 1; byte < sizeof(mh_z19_start)/sizeof(mh_z19_start[0])-1; byte++){
		if (!FIFO_IS_FULL(rxBuf)){
			FIFO_PUSH(txBuf, mh_z19_start[byte]);
		}
	}
	
	USART0.TXDATAL =  mh_z19_start[0];
	
	DEBUG_PORT.OUTCLR = DEBUG_PIN;
	DEBUG_PORT.OUTSET = DEBUG_PIN;

	uint16_t res = 0;
	while (FIFO_COUNT(rxBuf) != 2)
	#ifdef DEBUG
	{
		if (!(RESET_SOFT_PORT.IN & RESET_SOFT_PIN)) { //software reset
			_delay_ms(100);
			cli();
			while(!(RESET_SOFT_PORT.IN & RESET_SOFT_PIN));
			_PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRE_bm);//reset!
		}
		#endif
	}
	;
	USART0.CTRLB &= ~(USART_TXEN_bm | USART_RXEN_bm);
	USART0.CTRLA &= ~(USART_DREIF_bm | USART_RXCIE_bm);

	res = (uint16_t) FIFO_FRONT(rxBuf);
	FIFO_POP(rxBuf);
	res = (res << 8)+((uint16_t) FIFO_FRONT(rxBuf));
	FIFO_POP(rxBuf);
	return res;
}
