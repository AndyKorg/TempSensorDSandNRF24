/*
* ������� ������ �����������.
* �������� ������ �����. ������ ���� - �������������� ��� �������� �������������, ������ ���� - ��� �������, ���� ������ 0x01 - ������ ����������� �� ATTiny13
* � ��������� ��� ����� ����������� ���������� �� DS18B20. ���� ��� ������� ����������� �� ���������� SENSOR_NO = (0xfa00) - �������� ����������� ��������� �� ������ ��������� ds18b20
* ������� � ����� ������������ ������� ���������. ������������ ������� ���������� � ���� ����
* ����. ������ ���� ��� �������� ����������� ��� ������� WDT
* ������ � ������ ����� ��� ������� ������������ ������� WDT. ������� ���� �������� ���������� ������.
* ���� �������� �� ������� �� ������������ ������ ��������� �� ���������
* ������ ���� �� ���������:
* 		BufTx[0] = ATTINY_13A_8S_SLEEP;		//������� ��� ������� ���, ������� �� 8 ������
*		BufTx[1] = 0x00;					//������� ���� �������� ���
*		BufTx[2] = 0x01;					//������� ���� ��������
*		nRF_cmd_Write(nRF_W_ACK_PAYLOAD_MASK | 0b00000000, BUF_LEN, BufTx);	//�������� ����� ��� ������ 0
* Andy Korg (c) v.0.1 2014 �.
* http://radiokot.ru/circuit/digital/home/199/
*/


#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "ds18b20.h"
#include "nRF24L01P.h"
#include "avr/sleep.h"


#define SLEEP_WDT_S			8										//�������� ��� ������� WDT
#define SLEEP_PERIOD_10MIN	((10*60)/SLEEP_WDT_S)					//���������� ������������ WDT ��� ��������� ��� 10 �����
#define ATTEMPT_SEND_MAX	10										//������������ ���������� ������� ��������

#define	WDT_int_on()		WDTCR |= Bit(WDCE) | Bit(WDTIE)			//�������� ���������� �� ������

//ISR(wdt_reset){
//Interrupts from the watchdog timer, just a stub so that when you wake up, it does not go to reset
//}

volatile static uint8_t mode = nrf_send;

ISR(BUTTON_INT_VECT){
	if (BUTTON_PORT.INTFLAGS & BUTTON_PIN){
		mode = nrf_reg;
		BUTTON_PORT.INTFLAGS |= BUTTON_PIN;
	}
}

int main(void)
{
	uint8_t Attempt = ATTEMPT_SEND_MAX;
	uint16_t Tempr;
	
	PORTB.DIRSET = PIN3_bm;
	PORTB.DIRSET = PIN2_bm;
	PORTB.OUTCLR = PIN2_bm;
	PORTB.OUTSET = PIN2_bm;
	nrf_Response_t nRF_Answer;

	BUTTON_PORT.DIRCLR = BUTTON_PIN;
	BUTTON_INT = BUTTON_INT_TYPE | PORT_PULLUPEN_bm;
	
	//	PRR = (1<<PRTIM0) | (1<<PRADC);									//��������� ������ � ��� ��� �������� �������������
	//	wdt_reset();													//����� watchdog
	//	sei();
	
	//	nRF_Init();

	mode = nrf_reg;
	while(1){
		
		Tempr = GetTemperature(Attempt);
		if (Tempr != SENSOR_NO){
			PORTB.OUTCLR = PIN3_bm;
			PORTB.OUTSET = PIN3_bm;
		}
		uint8_t buf[4];
		buf[0] = DEVICE_TYPE_MH_Z19;
		buf[1] = DEVICE_TYPE_MH_Z19;
		buf[2] = (uint8_t)(Tempr>>8);
		buf[3] = (uint8_t)Tempr;
		nrf_err_t res = nRF_Send(mode, buf, 4, &nRF_Answer);
		if (res != nRF_OK){					//error
			if ((mode == nrf_reg) && (res == nRF_ERR_NO_REG_MODE) && nRF_real_address_is_set()){	//no reg mode and real address is set, send data
				mode = nrf_send;	//return to normal mode
				if (nRF_Send(nrf_send, buf, 4, &nRF_Answer) == nRF_OK){
					continue;
				}
			}
			Attempt--;												//����� ���� �������
			/*			if (Attempt==0){										//������� �������� ���������, ��������� �� 10 �������� �������� ��������
			nRF_Answer.Cmd = SLEEP_WDT_S;
			nRF_Answer.Data = SLEEP_PERIOD_10MIN;
			}
			else{													//������� ��� �� ��������� ���������� ����� 1 �������
			nRF_Answer.Cmd = 0;
			nRF_Answer.Data = SLEEP_PERIOD_10MIN;
			}
			*/
		}
		else {
			Attempt = ATTEMPT_SEND_MAX;
		}
		
		/*
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);						//����� ��������� ��� - ����������� ������ �� ������ ��� ������� ������ �� INT0
		WDTCR |= Bit(WDCE) | (nRF_Answer.Cmd & ((1<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0)));	//������� ���������
		for (u08 CountSleep = nRF_Answer.Data; CountSleep; CountSleep--){	//����
		PORTB = 0;
		PORTB = 0xff;											//��������� � ������� ��� �������� �������
		WDT_int_on();
		sleep_enable();
		sleep_cpu();
		sleep_disable();										//���������� -------------------------
		}
		*/
	}
}