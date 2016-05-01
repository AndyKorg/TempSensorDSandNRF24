/*
 * Работа с датчиком на шине 1-Ware
 * Не используются таймеры
 * ver. 1.0
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include "OneWare.h"

#define ONE_WARE_RESET_480_us		480								//Длительность импульса Reset на шине 1-Ware 480 мкс минимум, сделаю 560 как у samsung протокола
#define ONE_WARE_2_us				2								//Промежуток между таймслотами, обычно 1 мкс, довел до 2 мкс на всякий случай
#define ONE_WARE_60_us				60								//Максимальная длительность тайм-слота
#define ONE_WARE_240_us				240								//Максимальная длительность ожидания ответа на Reset
#define ONE_WARE_14_us				14								//Конец периода анализа ответного бита от устройства
#define ONE_WARE_5_us				5								//Максимальная длительность периода после таймслота чтения

#define ONE_WARE_INI()			ClearBit(ONE_WARE_PORT_OUT, ONE_WARE_PIN)	//Записать вывод 0 для прижатия к земле
#define ONE_WARE_NULL()			SetBit(ONE_WARE_DDR, ONE_WARE_PIN)			//Прижать к земле
#define ONE_WARE_ONE()			ClearBit(ONE_WARE_DDR, ONE_WARE_PIN)		//Отпустить
#define ONE_WARE_IS_NULL()		BitIsClear(ONE_WARE_PORT_IN, ONE_WARE_PIN)	//На входе низкий уровень
#define ONE_WARE_IS_ONE()		BitIsSet(ONE_WARE_PORT_IN, ONE_WARE_PIN)	//На входе высокий уровень


/************************************************************************/
/* Сброс шины Dallas 1-Ware, Возвращает 1 если есть ответ               */
/************************************************************************/
u08 OneWareReset(void){
	u08 Ret = 0;
	
	ONE_WARE_NULL();
	_delay_us(ONE_WARE_RESET_480_us);
	ONE_WARE_ONE();
	_delay_us(ONE_WARE_14_us);										//Подождать пока устройство среагирует
	for(u08 i=(ONE_WARE_240_us/ONE_WARE_2_us); i; i--){				//Ждем пока..
		_delay_us(ONE_WARE_2_us);									//Либо истечет период ожидания
		if ONE_WARE_IS_NULL(){										//либо устройство ответит
			Ret = 1;
			break;
		}
	}
	_delay_us(ONE_WARE_RESET_480_us);								//Ждем на шине еще 480 мкс от старта PRESENCE
	return Ret;
}

/************************************************************************/
/* Передать байт по шине Dallas                                         */
/************************************************************************/
void OneWareSendByte(u08 SendByte){
	u08 Count;

	for(Count=8; Count; Count--){
		ONE_WARE_NULL();
		_delay_us(ONE_WARE_2_us);									//Передать старт тайм-слота
		if (SendByte & 1)
			ONE_WARE_ONE();											//Передать 1
		SendByte >>= 1;
		_delay_us(ONE_WARE_60_us);									//Подождать длительность тайм-слота
		ONE_WARE_ONE();												//Промежуток между тайм-слотами
		_delay_us(ONE_WARE_2_us);
	}
}

/************************************************************************/
/* Принять байт по шине Dallas                                          */
/************************************************************************/
u08 OneWareReciveByte(void){
	u08 Count, Ret = 0;

	for(Count=0; Count<8; Count++){
		ONE_WARE_NULL();
		_delay_us(ONE_WARE_2_us);									//Передать старт тайм-слота
		ONE_WARE_ONE();												//Отпустить шину
		_delay_us(ONE_WARE_5_us);									//И подождать что бы устройство ответило
		Ret >>= 1;													//Подготовить следующий бит для приема
		if ONE_WARE_IS_ONE()										//ПРинять бит
			SetBit(Ret, 7);
		_delay_us(ONE_WARE_60_us);									//Подождать длительность тайм-слота
		_delay_us(ONE_WARE_2_us);									//Промежуток между тайм-слотами
	}
	return Ret;
}

void OneWareIni(void){
	SetBit(ONE_WARE_DDR, ONE_WARE_PIN);								//Настроить пин на вывод
	ClearBit(ONE_WARE_PORT_OUT, ONE_WARE_PIN);						//Записать вывод 0 для прижатия к земле
	ONE_WARE_NULL();												//Отпустить для готовности к работе
}