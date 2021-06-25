/*
\brief He is considered to be alone on the bus and therefore is not addressed
*/

#include "HAL.h"
#include <util/delay.h>
#include "OneWare.h"
#include "ds18b20.h"

//------------- Command DS18B20
#define DS18B20_START			0x44								//start measure
#define DS18B20_READ			0xbe								//read results

void crc_calc(uint8_t byte, uint8_t *crc){
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t tmp = 1 & (byte ^ (*crc));
		(*crc) >>= 1;
		byte >>= 1;
		if ( 0 != tmp ) (*crc) ^= 0x8c;
	}
}

uint16_t GetTemperature(uint8_t attempt){
	uint8_t Hi, Lo, crc = 0;

	OneWareIni();
	for (;attempt;attempt--){
		if (OneWareReset()){
			OneWareSendByte(ONE_WARE_SKIP_ROM);							//all devices
			OneWareSendByte(DS18B20_START);
			_delay_ms(100);												//wait result
			OneWareReset();
			OneWareSendByte(ONE_WARE_SKIP_ROM);
			OneWareSendByte(DS18B20_READ);
			Lo = OneWareReciveByte();//byte 0  Temperature LSB
			crc_calc(Lo, &crc);
			Hi = OneWareReciveByte();//Byte 1 Temperature MSB
			crc_calc(Hi, &crc);
			crc_calc(OneWareReciveByte(), &crc);;//Byte 2 TH Register or User Byte
			crc_calc(OneWareReciveByte(), &crc);;//Byte 3 TL Register or User Byte 2
			crc_calc(OneWareReciveByte(), &crc);;//Byte 4 Configuration Register
			crc_calc(OneWareReciveByte(), &crc);;//Byte 5 Reserved
			crc_calc(OneWareReciveByte(), &crc);;//Byte 6 Reserved
			crc_calc(OneWareReciveByte(), &crc);;//Byte 7 Reserved
			uint8_t crc_ctrl = OneWareReciveByte();
			OneWareReset();												//Remaining bytes are ignored
_delay_ms(100);//DEBUG
			if (crc == crc_ctrl){
				return ( ((uint16_t)Hi<<8) | ((uint16_t)Lo) );
			}
		}
	}
	return SENSOR_NO;
}
