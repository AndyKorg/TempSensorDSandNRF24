/*
 * OneWare.h
 * �� ������������ �������
 * ver 1.0
 */ 


#ifndef ONEWARE_H_
#define ONEWARE_H_

#include "avrlibtypes.h"
#include "bits_macros.h"

//------------- �������� ����� 1-Ware
#define ONE_WARE_DDR			DDRB								//������� ����������
#define ONE_WARE_PORT_OUT		PORTB								//������� ������
#define ONE_WARE_PORT_IN		PINB								//������� �����
#define ONE_WARE_PIN			PINB4								//���� ���������

//------------- ������� ���� 1-Ware
#define ONE_WARE_SEARCH_ROM		0xf0								//����� ������� - ������������ ��� ������������� ��������� ����������� ���������� � ������� ������������ ���������
#define ONE_WARE_READ_ROM		0x33								//������ ������ ���������� - ������������ ��� ����������� ������ ������������� ���������� �� ����
#define ONE_WARE_MATCH_ROM		0x55								//����� ������ - ������������ ��� ��������� � ����������� ������ ���������� �� ������ ������������
#define ONE_WARE_SKIP_ROM		0xcc								//������������ ����� - ������������ ��� ��������� �� ���� ����������� �����, ��� ���� ����� ���������� ������������ (����� ���������� � ������������ ����������)

void OneWareIni(void);												//������������� �����, ���� ��������
u08 OneWareReset(void);												//����� ���� � �������� ������ �� ����������. 1 - ���� ���������� �� ����, 0 - ����� �� �������
void OneWareSendByte(u08 SendByte);									//�������� ���� �� ����
u08 OneWareReciveByte(void);										//������� ���� �� ����

#endif /* ONEWARE_H_ */