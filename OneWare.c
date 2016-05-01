/*
 * ������ � �������� �� ���� 1-Ware
 * �� ������������ �������
 * ver. 1.0
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include "OneWare.h"

#define ONE_WARE_RESET_480_us		480								//������������ �������� Reset �� ���� 1-Ware 480 ��� �������, ������ 560 ��� � samsung ���������
#define ONE_WARE_2_us				2								//���������� ����� �����������, ������ 1 ���, ����� �� 2 ��� �� ������ ������
#define ONE_WARE_60_us				60								//������������ ������������ ����-�����
#define ONE_WARE_240_us				240								//������������ ������������ �������� ������ �� Reset
#define ONE_WARE_14_us				14								//����� ������� ������� ��������� ���� �� ����������
#define ONE_WARE_5_us				5								//������������ ������������ ������� ����� ��������� ������

#define ONE_WARE_INI()			ClearBit(ONE_WARE_PORT_OUT, ONE_WARE_PIN)	//�������� ����� 0 ��� �������� � �����
#define ONE_WARE_NULL()			SetBit(ONE_WARE_DDR, ONE_WARE_PIN)			//������� � �����
#define ONE_WARE_ONE()			ClearBit(ONE_WARE_DDR, ONE_WARE_PIN)		//���������
#define ONE_WARE_IS_NULL()		BitIsClear(ONE_WARE_PORT_IN, ONE_WARE_PIN)	//�� ����� ������ �������
#define ONE_WARE_IS_ONE()		BitIsSet(ONE_WARE_PORT_IN, ONE_WARE_PIN)	//�� ����� ������� �������


/************************************************************************/
/* ����� ���� Dallas 1-Ware, ���������� 1 ���� ���� �����               */
/************************************************************************/
u08 OneWareReset(void){
	u08 Ret = 0;
	
	ONE_WARE_NULL();
	_delay_us(ONE_WARE_RESET_480_us);
	ONE_WARE_ONE();
	_delay_us(ONE_WARE_14_us);										//��������� ���� ���������� ����������
	for(u08 i=(ONE_WARE_240_us/ONE_WARE_2_us); i; i--){				//���� ����..
		_delay_us(ONE_WARE_2_us);									//���� ������� ������ ��������
		if ONE_WARE_IS_NULL(){										//���� ���������� �������
			Ret = 1;
			break;
		}
	}
	_delay_us(ONE_WARE_RESET_480_us);								//���� �� ���� ��� 480 ��� �� ������ PRESENCE
	return Ret;
}

/************************************************************************/
/* �������� ���� �� ���� Dallas                                         */
/************************************************************************/
void OneWareSendByte(u08 SendByte){
	u08 Count;

	for(Count=8; Count; Count--){
		ONE_WARE_NULL();
		_delay_us(ONE_WARE_2_us);									//�������� ����� ����-�����
		if (SendByte & 1)
			ONE_WARE_ONE();											//�������� 1
		SendByte >>= 1;
		_delay_us(ONE_WARE_60_us);									//��������� ������������ ����-�����
		ONE_WARE_ONE();												//���������� ����� ����-�������
		_delay_us(ONE_WARE_2_us);
	}
}

/************************************************************************/
/* ������� ���� �� ���� Dallas                                          */
/************************************************************************/
u08 OneWareReciveByte(void){
	u08 Count, Ret = 0;

	for(Count=0; Count<8; Count++){
		ONE_WARE_NULL();
		_delay_us(ONE_WARE_2_us);									//�������� ����� ����-�����
		ONE_WARE_ONE();												//��������� ����
		_delay_us(ONE_WARE_5_us);									//� ��������� ��� �� ���������� ��������
		Ret >>= 1;													//����������� ��������� ��� ��� ������
		if ONE_WARE_IS_ONE()										//������� ���
			SetBit(Ret, 7);
		_delay_us(ONE_WARE_60_us);									//��������� ������������ ����-�����
		_delay_us(ONE_WARE_2_us);									//���������� ����� ����-�������
	}
	return Ret;
}

void OneWareIni(void){
	SetBit(ONE_WARE_DDR, ONE_WARE_PIN);								//��������� ��� �� �����
	ClearBit(ONE_WARE_PORT_OUT, ONE_WARE_PIN);						//�������� ����� 0 ��� �������� � �����
	ONE_WARE_NULL();												//��������� ��� ���������� � ������
}