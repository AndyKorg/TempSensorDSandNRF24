#include <stddef.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include "MHZ19.h"
#include "HAL.h"
#include "sleep_rtc.h"


#define MHZ19_BAUD	9600
#define USART0_BAUD_RATE(BAUD_RATE) ((64UL*F_CPU)/(16UL*BAUD_RATE))

#define MHZ19_PILOT_CMD_CODE	0xFF
#define MHZ19_READ_CMD_CODE		0x86
#define MHZ19_READ_CMD			{MHZ19_PILOT_CMD_CODE, 0x01, MHZ19_READ_CMD_CODE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}
#define MHZ19_ANSWER			{MHZ19_PILOT_CMD_CODE, MHZ19_READ_CMD_CODE}
const PROGMEM uint8_t read_cmd[] = MHZ19_READ_CMD;
const PROGMEM uint8_t answe_cmd[] = MHZ19_ANSWER;
#define MHZ19_READ_CMD_LEN		(sizeof(read_cmd)/sizeof(uint8_t))
#define MHZ19_READ_CMD_START	0
#define MHZ19_READ_CMD_NEXT		MHZ19_READ_CMD_START+1
#define MHZ19_READ_CRC_BYTE		MHZ19_READ_CMD_LEN-1
#define MHZ19_ANSWER_LEN		(sizeof(answe_cmd)/sizeof(uint8_t))
#define MHZ19_ANSWER_HIGHT_NUM	MHZ19_READ_CMD_START+2
#define MHZ19_ANSWER_LOW_NUM	MHZ19_READ_CMD_START+3
#define MHZ19_WARMING_UP_S		5//(3*60) //three minute warmin up

static bool usart_state_set(bool *value){
	static bool busy = false;
	bool i_state = isr_state();
	cli();
	if (value){
		busy = *value;
	}
	if (i_state){
		sei();
	}
	return busy;
}

static bool timeout_state(bool *value){
	static bool timeout = false;
	bool i_state = isr_state();
	cli();
	if (value){
		timeout = *value;
	}
	if (i_state){
		sei();
	}
	return timeout;
}

#define timeout_clear()	do {bool clear = false; timeout_state(&clear);} while (0)
#define usart_busy()	do {bool busy = true; usart_state_set(&busy); } while (0)
#define usart_free()	do {bool busy = false; usart_state_set(&busy); timeout_stop();} while (0)
#define usart_is_free()	(!usart_state_set(NULL))

bool usart_is_busy(void){
	return usart_state_set(NULL);
}

static uint16_t gas_value(uint16_t *value){
	static uint16_t gas_concentrat = 0;
	bool i_state = isr_state();
	cli();
	if (value){
		gas_concentrat = *value;
	}
	if (i_state){
		sei();
	}
	return gas_concentrat;
}

#if (SENSOR_TYPE == DEVICE_TYPE_MH_Z19)
void timeout_answer(void){
	bool timeout = true;
	timeout_state(&timeout);
	PORTB.OUTTGL = PIN4_bm;
}

ISR(USART0_DRE_vect){
	static uint8_t cmd_count = MHZ19_READ_CMD_START;//0 bytes are transmitted at the start of the measurement
	if (cmd_count != MHZ19_READ_CMD_LEN){
		USART0.TXDATAL = pgm_read_byte(&read_cmd[cmd_count]);
		cmd_count++;
		timeout_reset();
		return;
	}
	cmd_count = MHZ19_READ_CMD_START;
	USART0.CTRLA &= ~USART_DREIE_bm;//end command
}

ISR(USART0_RXC_vect){
	static uint8_t posCmd = MHZ19_READ_CMD_START, checksum = 0;
	uint8_t data = USART0.RXDATAL;
	static uint16_t result = 0;

	if (posCmd < MHZ19_ANSWER_LEN){
		if (data != pgm_read_byte(&answe_cmd[posCmd])){
			posCmd = MHZ19_READ_CMD_START;
			checksum = 0;
			return;
		}
	}
	else if (posCmd == MHZ19_ANSWER_HIGHT_NUM){
		result = ((uint16_t)data)<<8;
	}
	else if (posCmd == MHZ19_ANSWER_LOW_NUM){
		result = (result & 0xff00) | ((uint16_t)data);
	}
	else if (posCmd == MHZ19_READ_CRC_BYTE){
		uint8_t check = (~checksum)+1;
		if (check == data){
			gas_value(&result);
		}
		posCmd = MHZ19_READ_CMD_START;
		checksum = 0;
		usart_free();
		return;
	}
	if (posCmd){
		checksum += data;
	}
	posCmd++;
}
#endif

bool MHZ19_ready(uint8_t attempt){
	static bool already_init = false;
	
	if (!already_init){
		usart_free();
		USART0.BAUD = USART0_BAUD_RATE(MHZ19_BAUD);
		USART0.CTRLB |= USART_TXEN_bm | USART_RXEN_bm;
		USART0.CTRLA = USART_RXCIE_bm;
		mhz19_PortInit();
		already_init = true;
		PORTB.OUTTGL = PIN4_bm;	//DEBUG!
	}
	if (usart_is_free()){
		//timeout timer set
		mhz19_power_on();
		timeout_start(10, timeout_answer);//first start
		usart_busy();
		//reading attempt
		while (attempt){
			usart_busy();
			USART0.CTRLA |= USART_DREIE_bm;	//start read measure sensor
			timeout_clear();
			timeout_start(1, timeout_answer);
			while(usart_is_busy() && (!timeout_state(NULL)));//white timeout or read measure sensor
			if (usart_is_free()){
				timeout_stop();
				return true;
			}
			attempt--;
		}
	}
	timeout_stop();
	return false;
}

void MHZ19_stop(void){
	mhz19_power_off();
}

uint16_t GetCO2_MHZ19(uint8_t attempt){
	return gas_value(NULL);
}
