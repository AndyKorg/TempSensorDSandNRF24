/*
\brief driver nRF24L01P. Two mode - soft-SPI or hard-SPI
*/


#ifndef NRF24L01P_H_
#define NRF24L01P_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HAL.h"
#include "nRF24L01P Reg.h"

#define nRF_CHANNEL				2				//Number RF chanel
#define nRF_TYPE_SENSOR			SENSOR_TYPE		//type sensor
#define PTX_REG_MODE_LEN		(1 /*type*/ +1/*num.query*/ +5/*address*/)	//data size in PTX recording mode
#define PTX_REG_TYPE_BYTE		0				//number byte type devices from PTX
#define nRF_PRX_NO_REG_MODE		0xff			//prcs not in registration mode


typedef struct {								//answer PRX
	uint8_t Len;								//length answer
	uint8_t *Data;								//answer data
} nrf_Response_t;

#define nRF_REPEAT_MAX			7				//The number of retransmission attempts on failure. 0 - disable autorepeat
#define nRF_REPEAT_INTERVAL		nRF_RETR_500US	//Auto repeat interval

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
	nrf_send_mode = 0,				//sensor data transmission
	nrf_reg_mode,					//registration of the sensor
} nrf_oper_t;

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

void debug_off(void);

#endif /* NRF24L01P_H_ */