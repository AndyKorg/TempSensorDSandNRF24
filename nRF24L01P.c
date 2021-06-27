/*
\brief Sending a packet and receiving a response from the host via nRF24L01+
The first and second bytes are the value
The third byte code of the controlling microcontroller
*/


#include <stddef.h>
#include "nRF24L01P.h"

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

uint8_t nRF_cmd_Write(const uint8_t cmd, uint8_t Len, uint8_t *Data){
	
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

/*
\brief Initializes the nRF24L01 interface and transmits Data over the nRF_PIPE channel. High byte forward.

Returns 1 - if received normally, 0 if error.
If received normally, the nRF_Resp structure is full
*/
uint8_t nRF_Send(uint16_t Data, struct nRF_Response *nRF_Resp){

	uint8_t Buf[6], Status, Ret = 0;
	
	nRF_Init();
	
	Status = nRF_cmd_Write(nRF_FLUSH_RX, 0, NULL);				//clear FIFO buffers
	if (Status & (1<<nRF_0_ALLOWED)){	//module not responce
		return 0;
	}
	nRF_cmd_Write(nRF_FLUSH_TX, 0, NULL);
	Buf[0] = nRF_CHANNEL;	//number RF channel
	nRF_cmd_Write(nRF_WR_REG(nRF_RF_CH), 1, Buf);
	Buf[0] = (0 << nRF_RF_DR) | (1 << nRF_RF_PWR1) | (1 << nRF_RF_PWR0);	// 1 Mbps, TX gain: 0dbm
	nRF_cmd_Write(nRF_WR_REG(nRF_RF_SETUP), 1, Buf);

	Buf[0] = (0<<nRF_PRIM_RX) | (1<<nRF_PWR_UP) | (1<<nRF_EN_CRC) | (0<<nRF_CRCO);	//Transfer mode, turn on the board, one-byte CRC
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);

	Buf[0] = nRF_ADR_PIPE;								//адрес канала
	Buf[1] = nRF_PRE_ADR_PIPE;
	Buf[2] = nRF_PRE_ADR_PIPE;
	Buf[3] = nRF_PRE_ADR_PIPE;
	Buf[4] = nRF_PRE_ADR_PIPE;
	nRF_cmd_Write(nRF_WR_REG(nRF_TX_ADDR), 5, Buf);
	nRF_cmd_Write(nRF_WR_REG(nRF_RX_ADDR_P0), 5, Buf);//адрес канала 0 для приема ответа совпадает с адресом канала передачи
	
	Buf[0] = (1<<nRF_DPL_P5) |(1<<nRF_DPL_P4) |(1<<nRF_DPL_P3) |(1<<nRF_DPL_P2) |(1<<nRF_DPL_P1) |(1<<nRF_DPL_P0);//Allow dynamic packet length. Required for correct operation of Payload with ACK mode
	nRF_cmd_Write(nRF_WR_REG(nRF_DYNPD), 1, Buf);
	
	Buf[0] = (1<<nRF_EN_DPL) | (1<<nRF_EN_ACK_PAY);		//Enables Dynamic Payload Length and Enables Payload with ACK
	nRF_cmd_Write(nRF_WR_REG(nRF_FEATURE), 1, Buf);
	
	Buf[0] = nRF_REPEAT_INTERVAL | nRF_REPEAT_MAX;		//интервал автоповтора и количество попыток
	nRF_cmd_Write(nRF_WR_REG(nRF_SETUP_RETR), 1, Buf);

	Buf[0] = (1<<nRF_ERX_P0);
	nRF_cmd_Write(nRF_WR_REG(nRF_EN_RXADDR), 1, Buf);	//enable pipe 0 recive

	Buf[0] = nRF_RESERVED_BYTE;							//Собственно сам пакет для передачи
	Buf[1] = nRF_TMPR_ATTNY13_SENSOR;
	Buf[2] = (uint8_t)(Data >> 8);
	Buf[3] = (uint8_t)Data;
	nRF_cmd_Write(nRF_W_TX_PAYLOAD, nRF_SEND_LEN, Buf);	//Загрузить данные в передатчик

	nRF_GO();											//Начать передачу
	do{
		nRF_SELECT();
		nRF_ExchangeSPI(nRF_RD_REG(nRF_STATUS));
		Status = nRF_ExchangeSPI(nRF_NOP);
		nRF_DESELECT();
	}while((Status & nRF_IRQ_MASK) == 0);				//Ожидается либо окончание обмена либо ошибка обмена
	nRF_STOP();											//Остановить работу радиотракта


	Buf[0] = Status;									//Сбросить состояние
	nRF_cmd_Write(nRF_WR_REG(nRF_STATUS), 1, Buf);
	if (!nRF_TX_ERROR(Status)){							//Ошибки передачи нет
		if (Status & (1<<nRF_RX_DR)){					//Принят ответ от хоста
			nRF_SELECT();								//Читаем количество принятого
			nRF_ExchangeSPI(nRF_R_RX_PL_WID);
			Status = nRF_ExchangeSPI(nRF_NOP);
			nRF_DESELECT();
			if (Status == nRF_ACK_LEN){					//Длина ответа правильная
				nRF_SELECT();
				nRF_ExchangeSPI(nRF_R_RX_PAYLOAD);		//Читаем ответ из буфера
				(*nRF_Resp).Cmd = nRF_ExchangeSPI(nRF_NOP);//Команда
				(*nRF_Resp).Data = (uint16_t)nRF_ExchangeSPI(nRF_NOP);	//Старший байт параметра
				(*nRF_Resp).Data  = ((((*nRF_Resp).Data)<<8) & 0xff00) | (uint16_t)nRF_ExchangeSPI(nRF_NOP);//младший байт параметра
				nRF_DESELECT();
				Ret = 1;
			}
		}
	}
	Buf[0] = (0<<nRF_PWR_UP);							//Выключить плату
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);
	return Ret;
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
	nRF_PORT.DIRSET = nRF_SPI_MOSI | nRF_SPI_SCK | nRF_CE | nRF_CSN;
	nRF_SPI.CTRLA = SPI_MASTER_bm | SPI_ENABLE_bm;
	#endif
}