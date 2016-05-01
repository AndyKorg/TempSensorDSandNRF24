/*
 * ������ � ������� nRF24L01P ����� soft-spi
 *
 */ 


#ifndef NRF24L01P_H_
#define NRF24L01P_H_

#ifdef DEBUG
#define LedOn() PORTA  &= ~(1<<PORTA0)
#define LedOff() PORTA |= (1<<PORTA0)
#endif


#define nRF_PIPE				4				//����� ������������� ������
#define nRF_SEND_LEN			4				//����� ������������� ������ ������. ������ ��������� �� ��������� nRF_SEND_LEN � ����� ������������ ����� ....\ClockMatrix\nRF24L01P.h
#define nRF_TMPR_ATTNY13_SENSOR	0x01			//������ ����������� �� Attiny13. ������ ��������� �� ��������� nRF_TMPR_ATTNY13_SENSOR � ����� ������������ ����� ....\ClockMatrix\nRF24L01P.h
#define nRF_ACK_LEN				3				//����� ������ �� ����� ��� ������ ������ �� ������������. ������ ��������� �� ��������� nRF_ACK_LEN � ����� ������������ ����� ....\ClockMatrix\nRF24L01P.h
#define nRF_RESERVED_BYTE		0				//���� ����������������� ��� �������� �������������

#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "nRF24L01P Reg.h"
#include "bits_macros.h"

struct nRF_Response{							//����� �� �����������
	u08	Cmd;									//�������
	u16 Data;									//�������� �������
	};
	
#define nRF_NO_RECIV			0				//������ �� ������� ���������� ������ � ������.
#define nRF_REPEAT_MAX			7				//���������� ������� ������� �������� ��� �������. 0 - ��������� ����������
#define nRF_REPEAT_INTERVAL		nRF_RETR_500US	//�������� �����������
/*
#define nRF_MAX_TIMEOUT			((4UL*16UL)*2UL)//������������ ������������ �������� �������� ������ � ��. 4000 ��� = 4 �� - ������������ ������ ����� �������� ������ �� ��������, 16 - ������������ ���������� ��������. ��������� � 2 ���� ��� ����������
#define TIMER_DEVIDER			1024UL
#define TIMER_INTERVAL			(F_CPU/TIMER_DEVIDER/1000)	//�������� ��� �������� ��������� ��� ��������� ������� 1 ��

#if (TIMER_DEVIDER == 1024UL)
	#define TimerStart()		do {TCCR0A = (1<<WGM02) | (1<<WGM01); \
									TCCR0B = (0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);\
									TCNT0 = 0;} while(0)	//�������� ������
#else
	#error "TIMER_DEVIDER not valid"
#endif

#define TimerStop()				TCCR0B = (0<<CS02) | (0<<CS01) | (0<<CS00);	//��������� ������
*/

#if ((nRF_REPEAT_MAX<0) && (nRF_REPEAT_MAX>15))
#error "Incorrect value nRF_REPEAT_MAX. The nRF_REPEAT_MAX should be between 1 to 32"
#endif

#if ((nRF_ACK_LEN<1) && (nRF_ACK_LEN>32))
	#error "Incorrect value nRF_ACK_LEN. The nRF_ACK_LEN should be between 1 to 32"
#endif

#if (nRF_PIPE == 0)
	#define nRF_PRE_ADR_PIPE	0xe7
	#define nRF_ADR_PIPE		0xe7
#elif (nRF_PIPE == 1)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc2
#elif (nRF_PIPE == 2)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc3
#elif (nRF_PIPE == 3)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc4
#elif (nRF_PIPE == 4)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc5
#elif (nRF_PIPE == 5)
	#define nRF_PRE_ADR_PIPE	0xc2
	#define nRF_ADR_PIPE		0xc6
#else
	#error "Bad pipe! Pipe num from 0 to 5"
#endif

#if ((nRF_ACK_LEN<1) && (nRF_ACK_LEN>32))
	#error "Incorrect value nRF_ACK_LEN. The nRF_ACK_LEN should be between 1 to 32"
#endif

#define nRF_DDR					DDRB			//����������� SPI
#define nRF_PORTIN				PINB
#define nRF_PORT				PORTB
#define nRF_SPI_MOSI			PINB1
#define nRF_SPI_MISO			PINB3
#define nRF_SPI_SCK				PINB4
#define nRF_CSN					PINB0			//����� ���������
#define nRF_CE					PINB2			//������ ��������

#define nRF_SELECT()			ClearBit(nRF_PORT, nRF_CSN)
#define nRF_DESELECT()			SetBit(nRF_PORT, nRF_CSN)

#define nRF_GO()				SetBit(nRF_PORT, nRF_CE)
#define nRF_STOP()				ClearBit(nRF_PORT, nRF_CE)

//��������� ����������� WDT ��� ATTINY13A
#define ATTINY_13A_WDP0 0
#define ATTINY_13A_WDP1 1
#define ATTINY_13A_WDP2 2
#define ATTINY_13A_WDP3 5
#define ATTINY_13A_16MS_SLEEP	((0<<ATTINY_13A_WDP3) | (0<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	//������������ ������ ����� 16 ��
#define ATTINY_13A_05S_SLEEP	((0<<ATTINY_13A_WDP3) | (1<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	// -/- 0,5 ������
#define ATTINY_13A_1S_SLEEP		((0<<ATTINY_13A_WDP3) | (1<<ATTINY_13A_WDP2) | (1<ATTINY_13A_WDP1) | (0<<ATTINY_13A_WDP0))	// -/- 1 �������
#define ATTINY_13A_8S_SLEEP		((1<<ATTINY_13A_WDP3) | (0<<ATTINY_13A_WDP2) | (0<ATTINY_13A_WDP1) | (1<<ATTINY_13A_WDP0))	// -/- 8 ������

void nRF_Init(void);
u08 nRF_Send(u16 Data, struct nRF_Response *nRF_Resp);		//�������� ������ Data � ����� ������ nRF_Resp

#endif /* NRF24L01P_H_ */