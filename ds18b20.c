/*
 * ������������ ������� ds18b20
 * ��������� ��� �� ���� �� ���� � ������� �� ����������
 * ver. 1.0
 */ 

#include <util/delay.h>
#include "OneWare.h"
#include "ds18b20.h"

//------------- ������� DS18B20
#define DS18B20_START			0x44								//������ ��������� �����������
#define DS18B20_READ			0xbe								//��������� ������

/************************************************************************/
/* ���������� �������� ����������� ��� ���������� �������.              */
/************************************************************************/
u16 GetTemperature(void){
	u08 Hi, Lo;

	OneWareIni();
	if (OneWareReset()){
		OneWareSendByte(ONE_WARE_SKIP_ROM);							//��� ����������� �� ����
		OneWareSendByte(DS18B20_START);								//������ �������������� �����������
		_delay_ms(100);												//��������� ���� ����� ����� ���������
		OneWareReset();
		OneWareSendByte(ONE_WARE_SKIP_ROM);							//��������� ���������� �� ���� ����, �� �������� ������� ��� ���������
		OneWareSendByte(DS18B20_READ);
		Lo = OneWareReciveByte();									//������� ����
		Hi = OneWareReciveByte();									//������� ����
		OneWareReset();												//��������� ����� ������������
		return ( ((u16)Hi<<8) | ((u16)Lo) );
	}
	else
		return SENSOR_NO;
}
