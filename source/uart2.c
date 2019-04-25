#include "stc8f.h"
#include "uart2.h"


bit uart2_busy = 0;		//����2����æ��־
bit uart2_rc_ok = 0;	//����2������ɱ�־
bit uart2_wait_ack = 0;	//����2�ȴ�Ӧ���־
uchar uart2_rc_cnt = 0;	//����2���ռ���
uchar uart2_rc_interval = 0;	//����2���ݽ��ռ��
uchar xdata uart2_buf[UART2_BUF_SIZE] = 0;


// ����2��ʼ��
void Uart2Init()
{
	S2CON = 0x50;		//S2ST4��S2REN����ʹ��
	T2L = BRT2;
	T2H = BRT2 >> 8;
	AUXR |= T2R;		//
	AUXR |= T2x12;
	uart2_busy = 0;
}

// ����2��������
void Uart2Send(uchar* ptr, uchar len)
{
	uchar i;
	for(i = 0; i < len; i++)
	{
		while (uart2_busy);
		uart2_busy = 1;
		S2BUF = ptr[i];
	}
}

/* UART2�����жϷ���ģ��
void Uart2_Isr() interrupt 8 using 1
{
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;		//�巢���жϱ�־
		uart2_busy = 0;
    }
    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;     //������жϱ�־
		uart1_rc_interval = 0;
		if(uart2_rc_cnt < UART2_BUF_SIZE)	//��ֹ�������
		{
			uart2_buf[uart2_rc_cnt++] = S2BUF;
		}
    }
}
*/