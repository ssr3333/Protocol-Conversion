#include "stc8f.h"
#include "uart2.h"


bit uart2_busy = 0;		//串口2发送忙标志
bit uart2_rc_ok = 0;	//串口2接收完成标志
bit uart2_wait_ack = 0;	//串口2等待应答标志
uchar uart2_rc_cnt = 0;	//串口2接收计数
uchar uart2_rc_interval = 0;	//串口2数据接收间隔
uchar xdata uart2_buf[UART2_BUF_SIZE] = 0;


// 串口2初始化
void Uart2Init()
{
	S2CON = 0x50;		//S2ST4，S2REN接收使能
	T2L = BRT2;
	T2H = BRT2 >> 8;
	AUXR |= T2R;		//
	AUXR |= T2x12;
	uart2_busy = 0;
}

// 串口2发送数据
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

/* UART2串口中断服务模板
void Uart2_Isr() interrupt 8 using 1
{
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;		//清发送中断标志
		uart2_busy = 0;
    }
    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;     //清接收中断标志
		uart1_rc_interval = 0;
		if(uart2_rc_cnt < UART2_BUF_SIZE)	//防止数据溢出
		{
			uart2_buf[uart2_rc_cnt++] = S2BUF;
		}
    }
}
*/