/*
 * Передача пакета и прием ответа от хоста через nRF24L01+
 * Первый и второй байт - значение температуры или еще какой величины
 * Третий байт код управляющего микроконтролера Пока только 0x13 - Attiny13
 */ 

#include "nRF24L01P.h"

/************************************************************************/
/* Ввод- вывод байта по SPI. Возвращает значение прочитанное из MISO    */
/***************************************s********************************/
u08 nRF_ExchangeSPI(u08 Value){
	u08 Ret = 0;

	for(u08 i=8;i;i--){
		Ret <<= 1;
		if BitIsSet(nRF_PORTIN, nRF_SPI_MISO)			//Чтение MISO
			Ret |= 1;
		ClearBit(nRF_PORT, nRF_SPI_MOSI);
		if (Value & 0x80)								//Вывод MOSI
			SetBit(nRF_PORT, nRF_SPI_MOSI);
		Value <<= 1;
		SetBit(nRF_PORT, nRF_SPI_SCK);					//Записать бит
		ClearBit(nRF_PORT, nRF_SPI_SCK);
	}
	return Ret;
}

/************************************************************************/
/* Передача команды. Возвращает статус модуля							*/
/************************************************************************/
u08 nRF_cmd_Write(const u08 cmd, u08 Len, u08 *Data){
	u08 Status;
	
	nRF_SELECT();
	Status = nRF_ExchangeSPI(cmd);
	if (Len)
		for(u08 i=0;Len;Len--, i++)
			nRF_ExchangeSPI(*(Data+i));
	nRF_DESELECT();
	return Status;
}

/************************************************************************/
/* Инициализирует интерфейс nRF24L01 и передает Data					*/
/* по каналу nRF_PIPE. Старший байт вперед                              */
/* Возвращает 1 - если принято нормально, 0 если ошибка					*/
/* Если принято нормально то структура nRF_Resp заполнена				*/
/************************************************************************/
u08 nRF_Send(u16 Data, struct nRF_Response *nRF_Resp){

	u08 Buf[6], Status, Ret = 0;
	
	nRF_Init();
	
	nRF_cmd_Write(nRF_FLUSH_RX, 0, Buf);				//Очистить буфера FIFO
	nRF_cmd_Write(nRF_FLUSH_TX, 0, Buf);
	Buf[0] = (0<<nRF_PRIM_RX) | (1<<nRF_PWR_UP) | (1<<nRF_EN_CRC);	//Режим передачи, включить плату, двухбайтовый CRC - в принцепе можно обойтись однобайтовым, но фиг с ним пусть будет
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);

	Buf[0] = nRF_ADR_PIPE;								//адрес канала
	Buf[1] = nRF_PRE_ADR_PIPE;
	Buf[2] = nRF_PRE_ADR_PIPE;
	Buf[3] = nRF_PRE_ADR_PIPE;
	Buf[4] = nRF_PRE_ADR_PIPE;
	nRF_cmd_Write(nRF_WR_REG(nRF_TX_ADDR), 5, Buf);
	nRF_cmd_Write(nRF_WR_REG(nRF_RX_ADDR_P0), 5, Buf);//адрес канала 0 для приема ответа совпадает с адресом канала передачи
	
	Buf[0] = (1<<nRF_DPL_P5) |(1<<nRF_DPL_P4) |(1<<nRF_DPL_P3) |(1<<nRF_DPL_P2) |(1<<nRF_DPL_P1) |(1<<nRF_DPL_P0);//Разрешить динамическую длину пакетов
	nRF_cmd_Write(nRF_WR_REG(nRF_DYNPD), 1, Buf);
	
	Buf[0] = (1<<nRF_EN_DPL) | (1<<nRF_EN_ACK_PAY);		//Разрешить динамическую длину пакетов и пакет ответа
	nRF_cmd_Write(nRF_WR_REG(nRF_FEATURE), 1, Buf);
	
	Buf[0] = nRF_REPEAT_INTERVAL | nRF_REPEAT_MAX;		//интервал автоповтора и количество попыток
	nRF_cmd_Write(nRF_WR_REG(nRF_SETUP_RETR), 1, Buf);

	Buf[0] = nRF_RESERVED_BYTE;							//Собственно сам пакет для передачи
	Buf[1] = nRF_TMPR_ATTNY13_SENSOR;
	Buf[2] = (u08)(Data >> 8);
	Buf[3] = (u08)Data;
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
		if BitIsSet(Status, nRF_RX_DR){					//Принят ответ от хоста
			nRF_SELECT();								//Читаем количество принятого
			nRF_ExchangeSPI(nRF_R_RX_PL_WID);
			Status = nRF_ExchangeSPI(nRF_NOP);
			nRF_DESELECT();
			if (Status == nRF_ACK_LEN){					//Длина ответа правильная
				nRF_SELECT();
				nRF_ExchangeSPI(nRF_R_RX_PAYLOAD);		//Читаем ответ из буфера
				(*nRF_Resp).Cmd = nRF_ExchangeSPI(nRF_NOP);//Команда
				(*nRF_Resp).Data = (u16)nRF_ExchangeSPI(nRF_NOP);	//Старший байт параметра
				(*nRF_Resp).Data  = ((((*nRF_Resp).Data)<<8) & 0xff00) | (u16)nRF_ExchangeSPI(nRF_NOP);//младший байт параметра
				nRF_DESELECT();
				Ret = 1;
			}
		}
	}
	Buf[0] = (0<<nRF_PWR_UP);							//Выключить плату
	nRF_cmd_Write(nRF_WR_REG(nRF_CONFIG), 1, Buf);
	return Ret;
}

/************************************************************************/
/* Инициализация пинов управления nRF24L01                              */
/************************************************************************/
void nRF_Init(void){
	nRF_DDR =	(1<<nRF_CSN) |							//Выбор кристалла.
				(1<<nRF_SPI_MOSI) | (0<<nRF_SPI_MISO) | (1<<nRF_SPI_SCK) | //SPI интерфейс. SCK - Объединено с 1-ware!
				(1<<nRF_CE);							//Запуск обмена
	nRF_PORT =	(0<<nRF_CSN) |							//Кристалл не выбран
				(0<<nRF_SPI_MOSI) | (1<<nRF_SPI_MISO) |	//на выход 0, вход подтянуть к питанию
				(0<<nRF_SPI_SCK) |						//Тактовый импульс к 0
				(0<<nRF_CE);							//Модуль выключен
}