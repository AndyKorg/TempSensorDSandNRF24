#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HAL.h"
#include "usart.h"
#include "FIFO.h"

#define BAUD_RATE		115200
#define CMD_PILOT1		0x55
#define CMD_PILOT2		0xAA
#define CMD_PILOT_COUNT	4

#define UROUND(x) ((2UL*(x)+1)/2)

volatile FIFO(UART_BUF_LEN) txBuf, rxBuf, cmd;
static usart_cmd_cb func_cb;

static int my_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(
my_putchar,
NULL,
_FDEV_SETUP_WRITE
);

static bool usart_state_set(bool *value){
	static bool busy = false;
	if (value){
		bool i_state = isr_state();
		busy = *value;
		if (!i_state){
			cli();
		}
	}
	return busy;
}

#define usart_busy()	do {bool busy = true; usart_state_set(&busy);} while (0)
#define usart_free()	do {bool busy = false; usart_state_set(&busy);} while (0)

ISR(USART0_TXC_vect){
	if (FIFO_IS_EMPTY(txBuf)){
		usart_free();
	}
}

ISR(USART0_DRE_vect){
	if (!FIFO_IS_EMPTY(txBuf)){
		USART0.TXDATAL = FIFO_FRONT(txBuf);
		FIFO_POP(txBuf);
		return;
	}
	USART0.CTRLA &= ~USART_DREIE_bm;
}

ISR(USART0_RXC_vect){
	static uint8_t posCmd = 0;
	static cmd_t cmd;
	uint8_t data = USART0.RXDATAL;
	
	const uint8_t cmdPilot[CMD_PILOT_COUNT] = {CMD_PILOT1, CMD_PILOT2, CMD_PILOT1, CMD_PILOT2};
	if (posCmd < CMD_PILOT_COUNT ){
		cmd.cmd = CMD_EMPTY;
		if (data != cmdPilot[posCmd]){
			posCmd = 0;
			return;
		}
	}
	else if (posCmd == CMD_PILOT_COUNT){
		cmd.cmd = data;
	}
	else if (posCmd > (CMD_PILOT_COUNT)) {
		bool endcmd = false;
		//printf("%x\r", data);
		if ((posCmd-CMD_PILOT_COUNT-1) < MAX_DATA_CMD){
			cmd.data[posCmd - CMD_PILOT_COUNT-1] = data;
			if (data == 0) {
				endcmd = true;
			}
		}
		else {
			posCmd = 0;
		}
		if (endcmd){
			posCmd = 0;
			if (func_cb){
				func_cb(cmd);
			}
			return;
		}
	}
	posCmd++;
}

static int my_putchar(char c, FILE *stream){
	USART0.CTRLA &= ~USART_DREIE_bm;
	if (!FIFO_IS_FULL(txBuf)){
		FIFO_PUSH(txBuf, c);
	}
	USART0.CTRLA |= USART_DREIE_bm;
	usart_busy();
	return 0;
}

bool usart_is_busy(void){
	return usart_state_set(NULL);
}

bool usart_init(usart_cmd_cb func){
	func_cb = func;

	debugConPortInit();
	USART0.BAUD = (64UL*F_CPU)/(16*BAUD_RATE);
	
	USART0.CTRLB |= USART_TXEN_bm | USART_RXCIE_bm;
	USART0.CTRLA |= USART_TXCIF_bm | USART_DREIF_bm | USART_RXCIE_bm;
	PORTB.PIN3CTRL = PORT_PULLUPEN_bm;	//reduced consumption during sleep
	
	FIFO_FLUSH(txBuf);
	stdout = &mystdout;
	return true;
}
