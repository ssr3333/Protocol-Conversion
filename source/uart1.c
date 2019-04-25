#include "stc8f.h"
#include "uart1.h"


bit uart1_busy = 0;		//����1����æ��־
bit uart1_rc_ok = 0;	//��ʾ�յ���ȷվ�ţ������뼰��Ӧ���ȵ����ݣ�����δУ��
bit uart1_wait_ack = 0;	//����1�ȴ�Ӧ���־
uchar uart1_rc_cnt = 0;	//����1���ռ���
uchar uart1_rc_interval = 0;	//����1���ݽ��ռ��
uchar xdata uart1_buf[UART1_BUF_SIZE] = 0;


// ����1��ʼ��
void Uart1Init()
{
	P_SW1 = 0x00;
	SCON = 0x50;		//SM1��REN����ʹ��
	TL1 = BRT1;
	TH1 = BRT1 >> 8;
	TR1 = 1;
	AUXR |= T1x12;	   	//T1x12
	uart1_busy = 0;
}

// ����1��������
void Uart1Send(uchar* ptr, uchar len)
{
	uchar i;
	for(i = 0; i < len; i++)
	{
		while (uart1_busy);
		uart1_busy = 1;
		SBUF = ptr[i];
	}
}

/*
void UartIsr() interrupt 4 using 1	//����1�ж�
{
	if (TI)
	{
		TI = 0;
		uart1_busy = 0;
	}
	if (RI)
	{
		RI = 0;
		uart1_rc_interval = 0;
		if(uart1_rc_cnt < UART1_BUF_SIZE)	//��ֹ�������
		{
			uart1_buf[uart1_rc_cnt++] = SBUF;
		}
	}
}
*/