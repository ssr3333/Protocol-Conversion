#ifndef __UART1_H_
#define __UART1_H_


// ====================== �궨�� ====================== //
#ifndef FOSC
	#define	FOSC 11059200UL	//IRCƵ��
#endif
#define BAUD1 9600UL		//����1������
#define	BRT1 (65536 - FOSC / BAUD1 / 4) 	//����1�����ʿ���
#define UART1_BUF_SIZE 160	//����1�����С


extern bit uart1_busy;		//����1����æ��־
extern bit uart1_rc_ok;		//��ʾ����1֡���
extern bit uart1_wait_ack;	//����1�ȴ�Ӧ���־
extern unsigned char uart1_rc_cnt;	//����1���ռ���
extern unsigned char uart1_rc_interval;	//����1���ݽ��ռ��
extern unsigned char xdata uart1_buf[UART1_BUF_SIZE];


void Uart1Init();
void Uart1Send(unsigned char* ptr, unsigned char len);

#endif /* __UART1_H_ */