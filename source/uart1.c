#include "stc8f.h"
#include "uart1.h"


bit uart1_busy = 0;		//串口1发送忙标志
bit uart1_rc_ok = 0;	//表示收到正确站号，功能码及相应长度的数据，但还未校验
bit uart1_wait_ack = 0;	//串口1等待应答标志
uchar uart1_rc_cnt = 0;	//串口1接收计数
uchar uart1_rc_interval = 0;	//串口1数据接收间隔
uchar xdata uart1_buf[UART1_BUF_SIZE] = 0;


// 串口1初始化
void Uart1Init()
{
	P_SW1 = 0x00;
	SCON = 0x50;		//SM1，REN接收使能
	TL1 = BRT1;
	TH1 = BRT1 >> 8;
	TR1 = 1;
	AUXR |= T1x12;	   	//T1x12
	uart1_busy = 0;
}

// 串口1发送数据
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
void UartIsr() interrupt 4 using 1	//串口1中断
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
		if(uart1_rc_cnt < UART1_BUF_SIZE)	//防止数据溢出
		{
			uart1_buf[uart1_rc_cnt++] = SBUF;
		}
	}
}
*/