#ifndef __UART2_H_
#define __UART2_H_


// ====================== �궨�� ====================== //
#ifndef FOSC
	#define	FOSC 11059200UL	//IRCƵ��
#endif

#define BAUD2 9600UL		//����2������
#define	BRT2 (65536 - FOSC / BAUD2 / 4) 	//����2�����ʿ���
#define UART2_BUF_SIZE 160	//����2�����С


extern bit uart2_busy;		//����2����æ��־
extern bit uart2_rc_ok;		//��ʾ����1֡���
extern bit uart2_wait_ack;	//����2�ȴ�Ӧ���־
extern unsigned char uart2_rc_cnt;	//����2���ռ���
extern unsigned char uart2_rc_interval;	//����2���ݽ��ռ��
extern unsigned char xdata uart2_buf[UART2_BUF_SIZE];


void Uart2Init();
void Uart2Send(unsigned char* ptr, unsigned char len);

#endif /* __UART2_H_ */