/*
\brief driver nRF24L01P. Two mode - soft-SPI or hard-SPI
*/


#ifndef NRF24L01P_H_
#define NRF24L01P_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "external_device.h"
#include "HAL.h"
#include "nRF24L01P Reg.h"

#define nRF_CHANNEL				2				//Number RF chanel
#define nRF_PIPE				1				//number pipe
#define nRF_SEND_LEN			4				//����� ������������� ������ ������. ������ ��������� �� ��������� nRF_SEND_LEN � ����� ������������ ����� ....\ClockMatrix\nRF24L01P.h
#define nRF_TYPE_SENSOR			DEVICE_TYPE_MH_Z19	//type sensor
#define nRF_ACK_LEN				3				//����� ������ �� ����� ��� ������ ������ �� ������������. ������ ��������� �� ��������� nRF_ACK_LEN � ����� ������������ ����� ....\ClockMatrix\nRF24L01P.h
#define nRF_RESERVED_BYTE		0				//���� ����������������� ��� �������� �������������

typedef struct {								//answer PRX
	uint8_t Len;								//length answer
	uint8_t *Data;								//answer data
} nrf_Response_t;

#define nRF_NO_RECIV			0				//������ �� ������� ���������� ������ � ������.
#define nRF_REPEAT_MAX			7				//���������� ������� ������� �������� ��� �������. 0 - ��������� ����������
#define nRF_REPEAT_INTERVAL		nRF_RETR_500US	//�������� �����������

typedef enum {
	nRF_OK = 0,
	nRF_ERR_NO_MODULE,			//module nRF not answer
	nRF_ERR_NO_ANSWER,			//PRX not answer
	nRF_ERR_NO_REG_MODE,		//PRX not registration mode 
	nRF_ERR_ADDR_NOT_FOUND,		//not registered real address
	nRF_ERR_DATA_IS_EMPTY,		//length data is 0
} nrf_err_t;

/*
\brief type of transaction
*/
typedef enum{
	nrf_send = 0,				//sensor data transmission
	nrf_reg,					//registration of the sensor
} nrf_oper_t;

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

void nRF_Init(void);

/*
\brief Data transmission and reception of nRF_Resp response over a given pipe.
Parameters:
oper - type operation.
Data - data for send
nRf_Resp - answer data
Return:
if nRF_OK is successfully transmitted and a response is received. The answer is in the nRF_Resp, the answer can be of zero length.
Memory capture for storing the response occurs at this location, but freeing memory lies with the programmer.
nRF_ERR_NO_MODULE - if module nRF not answer
nRF_ERR_NO_ANSWER - if PRX not answer
nRF_ERR_NO_REG_MODE - not registration mode or another sensor is being registered
In case of errors, the memory for the response is not allocated and is not captured.
*/
nrf_err_t nRF_Send(const nrf_oper_t oper, const uint8_t *data, const uint8_t len, nrf_Response_t *nRF_Resp);

/*
\brief returns true if PTX have real address
*/

bool nRF_real_address_is_set(void);

#endif /* NRF24L01P_H_ */