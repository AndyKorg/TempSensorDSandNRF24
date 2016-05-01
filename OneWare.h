/*
 * OneWare.h
 * Ќе используютс€ таймеры
 * ver 1.0
 */ 


#ifndef ONEWARE_H_
#define ONEWARE_H_

#include "avrlibtypes.h"
#include "bits_macros.h"

//------------- ќписание порта 1-Ware
#define ONE_WARE_DDR			DDRB								//–егистр управлени€
#define ONE_WARE_PORT_OUT		PORTB								//–егистр вывода
#define ONE_WARE_PORT_IN		PINB								//–егистр ввода
#define ONE_WARE_PIN			PINB4								//Ќога протокола

//-------------  оманды шины 1-Ware
#define ONE_WARE_SEARCH_ROM		0xf0								//ѕоиск адресов - используетс€ при универсальном алгоритме определени€ количества и адресов подключенных устройств
#define ONE_WARE_READ_ROM		0x33								//„тение адреса устройства - используетс€ дл€ определени€ адреса единственного устройства на шине
#define ONE_WARE_MATCH_ROM		0x55								//¬ыбор адреса - используетс€ дл€ обращени€ к конкретному адресу устройства из многих подключенных
#define ONE_WARE_SKIP_ROM		0xcc								//»гнорировать адрес - используетс€ дл€ обращени€ ко всем устройствам сразу, при этом адрес устройства игнорируетс€ (можно обращатьс€ к неизвестному устройству)

void OneWareIni(void);												//»нициализаци€ порта, пока пустышка
u08 OneWareReset(void);												//—брос шины и ожидание ответа от устройства. 1 - есть устройство на шине, 0 - никто не ответил
void OneWareSendByte(u08 SendByte);									//ѕередать байт по шине
u08 OneWareReciveByte(void);										//ѕрин€ть байт по шине

#endif /* ONEWARE_H_ */