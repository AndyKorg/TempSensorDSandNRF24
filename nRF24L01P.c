/*
\brief Sending a packet and receiving a response from the host via nRF24L01+
The first and second bytes are the value
The third byte code of the controlling microcontroller
*/


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>

#include "nRF24L01P.h"

#define ADDR_LEN 				5	//address lenght device, while fixed

#define REG_ATTEMPT_MAX			10	//maximum number attempt registration
#define REG_ADDRESS				{0xe7,0xe7,0xe7,0xe7,0xe7}	//default address for registraion PTX mode
#define PTX_REG_MODE_NULL_ADR	0,0,0,0,0	//zero address

#define PTX_REG_TYPE_BYTE		0	//number byte type devices from PTX
#define PTX_REG_QUERY_NUM_BYTE	1	//number byte number query
#define PTX_REG_ADR_START_BYTE	2	//starting byte for device address

#define PTX_REG_MODE_NO_TYPE	0	//type sensor not set
#define PTX_REG_MODE_QUERY_0	0	//number query for start process
#define PTX_REG_MODE_QUERY_1	1	//number query for end process
#define PTX_REG_MODE_NULL_ADR	0,0,0,0,0	//zero address

#if __AVR_ARCH__ >= 100
#define ClearBit(reg, bit)	do {reg.OUTCLR = bit;} while (0)
#define SetBit(reg, bit)	do {reg.OUTSET = bit;} while (0)
#define	BitIsSet(reg, bit)	(reg.IN & bit)
#else
#define ClearBit(reg, bit)	do {reg &= ~(1<<bit);} while (0)
#define SetBit(reg, bit)	do {reg |= (1<<bit);} while (0)
#define BitIsSet(reg, bit)	(reg & (1<<bit))
#endif

#define nRF_SELECT()			ClearBit(nRF_PORT, nRF_CSN)
#define nRF_DESELECT()			SetBit(nRF_PORT, nRF_CSN)

#define nRF_GO()				SetBit(nRF_PORT, nRF_CE)
#define nRF_STOP()				ClearBit(nRF_PORT, nRF_CE)

typedef enum {
	stProcess = 0,	//the process of obtaining the address is in progress
	stReal,			//address assigned
} state_adr_t;

typedef struct
{
	uint8_t adr[ADDR_LEN];
	state_adr_t state;
} address_t;

static address_t real_address = {{PTX_REG_MODE_NULL_ADR}, stProcess};


/*
\brief pipe and address register control pipe
*/
typedef enum {
	nrf24_pipe0 = nRF_RX_ADDR_P0,
	nrf24_pipe1 = nRF_RX_ADDR_P1,
	nrf24_pipe2 = nRF_RX_ADDR_P2,
	nrf24_pipe3 = nRF_RX_ADDR_P3,
	nrf24_pipe4 = nRF_RX_ADDR_P4,
	nrf24_pipe5 = nRF_RX_ADDR_P5,
	nrf24_pipeTx = nRF_TX_ADDR,
	nrf24_pipeMax,
} nrf_pipe_t;

/*
\brief I/O bytes over SPI. Returns the value read from MISO
*/
uint8_t nRF_ExchangeSPI(uint8_t value){
	uint8_t Ret = 0;

	#ifdef nRF_SPI_SOFT
	for(uint8_t i=8;i;i--){
		Ret <<= 1;
		if BitIsSet(nRF_PORTIN, nRF_SPI_MISO)			//read MISO
		Ret |= 1;
		ClearBit(nRF_PORT, nRF_SPI_MOSI);
		if (value & 0x80)								//out MOSI
		SetBit(nRF_PORT, nRF_SPI_MOSI);
		value <<= 1;
		SetBit(nRF_PORT, nRF_SPI_SCK);					//write bit
		ClearBit(nRF_PORT, nRF_SPI_SCK);
	}
	#else
	nRF_SPI.DATA = value;
	while (!(nRF_SPI.INTFLAGS & SPI_RXCIF_bm));
	Ret = nRF_SPI.DATA;
	#endif

	return Ret;
}

/*
\brief Command transmission. Returns the status of the module
*/

uint8_t nRF_cmd_Write(const uint8_t cmd, uint8_t Len, const uint8_t *Data){
	
	nRF_SELECT();
	uint8_t Status = nRF_ExchangeSPI(cmd);
	if (Len){
		for(uint8_t i=0;Len;Len--, i++){
			nRF_ExchangeSPI(*(Data+i));
		}
	}
	nRF_DESELECT();
	return Status;
}

bool nRF_real_address_is_set(void){
	uint8_t adr_or = 0, tmp;
	for (tmp = 0; tmp<ADDR_LEN;tmp++){
		adr_or |= *(real_address.adr+tmp);
	}
	return adr_or & real_address.state;
}
/*
\brief Initializes the nRF24L01 interface and transmits Data over the nRF_PIPE channel. High byte forward.

Returns nRF_OK - if received normally, nRF_ERR_NO_MODULE if module nRF24 not found.
If received normally, the nRF_Resp structure is full
*/
nrf_err_t send(const uint8_t *adr, const uint8_t *data, const uint8_t len, nrf_Response_t *nRF_Resp){

	uint8_t Buf[6], tmp;
	nrf_err_t Ret = nRF_ERR_NO_ANSWER;
	
	tmp = nRF_cmd_Write(nRF_FLUSH_RX, 0, NULL);				//clear FIFO buffers
	if (tmp & (1<<nRF_0_ALLOWED)){	//module not responce
		return nRF_ERR_NO_MODULE;
	}
	if ((!len) || (!data)) {
		return nRF_ERR_DATA_IS_EMPTY;
	}
	Buf[0] = nRF_CHANNEL;	//number RF channel
	nRF_cmd_Write(nRF_WR_REG(nRF_RF_CH), 1, Buf);
	Buf[0] = (0 << nRF_RF_DR) | (1 << nRF_RF_PWR1) | (1 << nRF_RF_PWR0);	// 1 Mbps, TX gain: 0dbm
	nRF_cmd_Write(nRF_WR_REG(nRF_RF_SETUP), 1, Buf);

	Buf[0] = (0<<nRF_PRIM_RX) | (1<<nRF_PWR_UP) | (1<<nRF_EN_CRC) | (0<<nRF_CRCO);	//Transfer mode, turn on the board, one-byte CRC
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);

	Buf[0] = (1<<nRF_DPL_P5) |(1<<nRF_DPL_P4) |(1<<nRF_DPL_P3) |(1<<nRF_DPL_P2) |(1<<nRF_DPL_P1) |(1<<nRF_DPL_P0);//Allow dynamic packet length. Required for correct operation of Payload with ACK mode
	nRF_cmd_Write(nRF_WR_REG(nRF_DYNPD), 1, Buf);
	
	Buf[0] = (1<<nRF_EN_DPL) | (1<<nRF_EN_ACK_PAY);		//Enables Dynamic Payload Length and Enables Payload with ACK
	nRF_cmd_Write(nRF_WR_REG(nRF_FEATURE), 1, Buf);
	
	Buf[0] = nRF_REPEAT_INTERVAL | nRF_REPEAT_MAX;		//autorepeat interval and number of attempts
	nRF_cmd_Write(nRF_WR_REG(nRF_SETUP_RETR), 1, Buf);

	Buf[0] = (1<<nRF_ERX_P0);
	nRF_cmd_Write(nRF_WR_REG(nRF_EN_RXADDR), 1, Buf);	//enable pipe 0 recive

	//the address of channel 0 for receiving the response is the same as the address of the transmitting channel
	nRF_cmd_Write(nRF_WR_REG(nRF_TX_ADDR), ADDR_LEN, adr);
	nRF_cmd_Write(nRF_WR_REG(nRF_RX_ADDR_P0), ADDR_LEN, adr);
	nRF_cmd_Write(nRF_FLUSH_TX, 0, NULL);				//Clear FIFO TX buffer
	nRF_cmd_Write(nRF_W_TX_PAYLOAD, len, data);

	nRF_GO();											//start transfer
	do{
		nRF_SELECT();
		nRF_ExchangeSPI(nRF_RD_REG(nRF_STATUS));
		tmp = nRF_ExchangeSPI(nRF_NOP);
		nRF_DESELECT();
	}while((tmp & nRF_IRQ_MASK) == 0);					//Either the end of the exchange or an exchange error is expected
	nRF_STOP();											//stop transfer


	Buf[0] = tmp;										//reset state
	nRF_cmd_Write(nRF_WR_REG(nRF_STATUS), 1, Buf);
	if (!nRF_TX_ERROR(tmp)){							//error not found
		if (tmp & (1<<nRF_RX_DR)){						//Принят ответ от хоста. TODO:Если ответа нет, то нужно переспросить
			nRF_SELECT();								//read length answer
			nRF_ExchangeSPI(nRF_R_RX_PL_WID);
			tmp = nRF_ExchangeSPI(nRF_NOP);
			nRF_DESELECT();
			nRF_Resp->Len = tmp;
			if (tmp){
				if (nRF_Resp->Data){
					free(nRF_Resp->Data);
				}
				nRF_Resp->Data = malloc(tmp);
				if (nRF_Resp->Data){
					nRF_SELECT();
					nRF_ExchangeSPI(nRF_R_RX_PAYLOAD);		//read answer
					for(tmp = 0; tmp < nRF_Resp->Len; tmp++){
						*(nRF_Resp->Data+tmp) = (uint16_t)nRF_ExchangeSPI(nRF_NOP);
					}
					nRF_DESELECT();
				}
				Ret = nRF_OK;
			}
		}
	}
	Buf[0] = (0<<nRF_PWR_UP);							//Выключить плату
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);
	return Ret;
}

/*
\brief Initializes the nRF24L01 interface and transmits Data over the nRF_PIPE channel. High byte forward.

Returns nRF_OK - if received normally, nRF_ERR_NO_MODULE if module nRF24 not found.
If received normally, the nRF_Resp structure is full
*/
nrf_err_t nRF_Send(const nrf_oper_t oper, const uint8_t *data, const uint8_t len, nrf_Response_t *nRF_Resp){
	
	if (oper == nrf_reg_mode){
		//registartion mode - default address pip0
		uint8_t reg_adr[ADDR_LEN] = REG_ADDRESS;
		uint8_t reg_buf[PTX_REG_MODE_LEN*3];
		reg_buf[PTX_REG_TYPE_BYTE] = nRF_TYPE_SENSOR;
		reg_buf[PTX_REG_QUERY_NUM_BYTE] = PTX_REG_MODE_QUERY_0;
		memcpy(reg_buf+PTX_REG_ADR_START_BYTE, real_address.adr, ADDR_LEN);	//send current address
		uint8_t attempt = REG_ATTEMPT_MAX;
		nrf_err_t ret;
		while (attempt){
			ret = send(reg_adr, reg_buf, PTX_REG_MODE_LEN, nRF_Resp);
			if (ret == nRF_OK){
				if (nRF_Resp->Len == PTX_REG_MODE_LEN){		//length correct
					if ((*(nRF_Resp->Data+PTX_REG_TYPE_BYTE) == PTX_REG_MODE_NO_TYPE) && (*(nRF_Resp->Data+PTX_REG_QUERY_NUM_BYTE) == PTX_REG_MODE_QUERY_0)){
						//white next query from PTX
						reg_buf[PTX_REG_QUERY_NUM_BYTE] = PTX_REG_MODE_QUERY_1;
						real_address.state = stProcess;
						continue;
					}
					else if ((*(nRF_Resp->Data+PTX_REG_TYPE_BYTE) == nRF_TYPE_SENSOR) && (*(nRF_Resp->Data+PTX_REG_QUERY_NUM_BYTE) == PTX_REG_MODE_QUERY_1)){
						//answer address sensor
						memcpy(real_address.adr, nRF_Resp->Data+PTX_REG_ADR_START_BYTE, ADDR_LEN);
						real_address.state = stReal;
						eeprom_write_block(&real_address, (void*) nRF_EEPROM, sizeof(address_t));
					}
					return nRF_OK;	//possible not registration mode or another sensor is being registered. check nRF_Resp
				}
			}
			else if (ret == nRF_ERR_NO_MODULE){	//module not set. we will not try to transmit.
				return ret;
			}
			attempt--;
		}
		return ret;
	}
	//data send mode
	if (!nRF_real_address_is_set()){
		return nRF_ERR_ADDR_NOT_FOUND;
	}
	return send(real_address.adr, data, len, nRF_Resp);
}

void nRF_Init(void){
	#ifdef nRF_SPI_SOFT
	#if __AVR_ARCH__ >= 100
	nRF_DDR = nRF_CSN | nRF_SPI_MOSI | nRF_SPI_SCK | nRF_CE;
	#else
	nRF_DDR = (1<<nRF_CSN) | (1<<nRF_SPI_MOSI) | (0<<nRF_SPI_MISO) | (1<<nRF_SPI_SCK) | (1<<nRF_CE);
	#endif
	nRF_DESELECT();
	#else
	//tinyAVR® 1-series
	nRF_PORT.OUTSET = nRF_SPI_MOSI | nRF_SPI_SCK | nRF_CE | nRF_CSN;
	nRF_PORT.DIRSET = nRF_SPI_MOSI | nRF_SPI_SCK | nRF_CE | nRF_CSN;
	nRF_PORT.nRF_SPI_MISO_CRL = PORT_PULLUPEN_bm;	//reduced consumption during sleep
	nRF_SPI.CTRLA = SPI_MASTER_bm | SPI_ENABLE_bm;
	#endif
	address_t buf;
	eeprom_read_block(&buf, (void*) nRF_EEPROM, sizeof(address_t));
	real_address.state = stProcess;
	if (buf.state == stReal){
		memcpy(&real_address, &buf, sizeof(address_t));
	}
}

void debug_off(void){
	nRF_STOP();											//stop transfer
	nRF_DESELECT();
	uint8_t Buf[2];
	Buf[0] = (0<<nRF_PWR_UP);							//Выключить плату
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);
}