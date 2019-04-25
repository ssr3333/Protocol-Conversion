#ifndef __IAP_H_
#define __IAP_H_

/* 1Tϵ�мĴ�����ַ
sfr     IAP_DATA    =   0xC2;	//���ݼĴ���
sfr     IAP_ADDRH   =   0xC3;	//��ַ���ֽ�
sfr     IAP_ADDRL   =   0xC4;	//��ַ���ֽ�
sfr     IAP_CMD     =   0xC5;	//����ģʽ
sfr     IAP_TRIG    =   0xC6;	//������Ĵ���
sfr     IAP_CONTR   =   0xC7;	//���ƼĴ���
*/

//��ͬʱ��Ƶ���µ�IAP�����ȴ�ʱ������
//���úú���Ӳ����ɣ����������ʱ
#define WT_30M          0x80 	//SYSCLK < 30MHz
#define WT_24M          0x81	//SYSCLK < 24MHz
#define WT_20M          0x82	//SYSCLK < 20MHz
#define WT_12M          0x83	//SYSCLK < 12MHz
#define WT_6M           0x84	//SYSCLK < 6MHz
#define WT_3M           0x85	//SYSCLK < 3MHz
#define WT_2M           0x86	//SYSCLK < 2MHz
#define WT_1M           0x87	//SYSCLK < 1MHz


void IapIdle();	//�ر�IAP
char IapRead(int addr);	//IAP��
void IapWrite(int addr, char dat); //IAPд
void IapErase(int addr);//IAP����

#endif /*__IAP_H_*/