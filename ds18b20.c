/*
 * Обслуживание датчика ds18b20
 * Считается что он один на шине и поэтому не адресуется
 * ver. 1.0
 */ 

#include <util/delay.h>
#include "OneWare.h"
#include "ds18b20.h"

//------------- Команды DS18B20
#define DS18B20_START			0x44								//Начать измерение температуры
#define DS18B20_READ			0xbe								//Прочитать память

/************************************************************************/
/* Возвращает значение температуры или отсутствие датчика.              */
/************************************************************************/
u16 GetTemperature(void){
	u08 Hi, Lo;

	OneWareIni();
	if (OneWareReset()){
		OneWareSendByte(ONE_WARE_SKIP_ROM);							//Все устройствам на шине
		OneWareSendByte(DS18B20_START);								//Начать преобразование температуры
		_delay_ms(100);												//Подождать пока будет готов результат
		OneWareReset();
		OneWareSendByte(ONE_WARE_SKIP_ROM);							//Поскольку устройство на шине одно, то посылаем команду без адресации
		OneWareSendByte(DS18B20_READ);
		Lo = OneWareReciveByte();									//Младший байт
		Hi = OneWareReciveByte();									//Старший байт
		OneWareReset();												//Остальные байты игнорируются
		return ( ((u16)Hi<<8) | ((u16)Lo) );
	}
	else
		return SENSOR_NO;
}
