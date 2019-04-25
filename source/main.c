// ==================== 引用头文件 ==================== //
#include "stc8f.h"
#include "sms310.h"	//短信模块
#include "iap.h"	//EEPROM操作    
#include "uart1.h"	//串口1
#include "uart2.h"	//串口2
#include "usr_gprs_730.h"	//USR_GPRS_730 DTU模块
#include "pcb_rs485convertor180621.h" //转换板定义


// ====================== 宏定义 ====================== //
//#define	FOSC	11059200UL	//IRC频率


// ===================== 全局变量 ===================== //
uint t1_cnt = 0;		//定时器1用计数
uchar work_interval = 0;//while 工作间隔
uchar get_result = 0;	//得到结果
uchar wait_delay = 0;	//等待应答延时



// ====================== 子函数 ====================== //
// 软件延时，单位10ms@11.0592MHz
void Delay10ms(unsigned char delay)	
{
	unsigned char i, j;
	do
	{
		i = 108;
		j = 145;
		do
		{
			while (--j);
		} while (--i);
	} while (--delay);
}

// 定时器1参数配置	1毫秒@11.0592MHz 当有串口1初始化时，部分参数已被配置
void Timer1Init(void)		
{
//	AUXR |= T1x12;	//定时器时钟1T模式
//	TMOD &= 0x0F;	//设置定时器模式
//	TL1 = 0xCD;		//设置定时初值
//	TH1 = 0xD4;		//设置定时初值
	TF1 = 0;		//清除TF1标志
//	TR1 = 1;		//定时器1开始计时
//	ET1 = 1;		//使能定时器1中断
}



// ===================== 中断服务 ===================== //
void TM1_Isr() interrupt 3 using 1	//定时器1中断
{
	TF1 = 0;
	//9600波特率下每次中断间隔约26us
	if(t1_cnt++ > 38400)	//26us * 38400 = 1s
	{
		t1_cnt = 0;

		if(work_interval > 0)
		{
			work_interval--;
		}
	}

	//串口1字符间隔，用于判断一帧结束
	//9600波特率下，1起始位，8数据位，1停止位，
	//每比特时间1000000÷9600=104.2us
	//每字符时间1000000÷9600×10=1041.7us
	//以3.5倍间隔时间为帧间隔
	//1000000÷9600×10×3.5=3645.8us
	if((uart1_rc_cnt > 0) && (uart1_rc_ok == 0))
	{
		uart1_rc_interval++;
		if(uart1_rc_interval > 140)
		{
			uart1_rc_interval = 0;
			uart1_rc_ok = 1;
		}
	}

	//串口2字符间隔，用于判断一帧结束
	//9600波特率下，1起始位，8数据位，1停止位，
	//每比特时间1000000÷9600=104.2us
	//每字符时间1000000÷9600×10=1041.7us
	//以3.5倍间隔时间为帧间隔
	//1000000÷9600×10×3.5=3645.8us
	if((uart2_rc_cnt > 0) && (uart2_rc_ok == 0))
	{
		uart2_rc_interval++;
		if(uart2_rc_interval > 140)
		{
			uart2_rc_interval = 0;
			uart2_rc_ok = 1;
		}
	}
}

void Uart1Isr() interrupt 4 using 1	//串口1中断
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
		uart2_rc_interval = 0;
		if(uart2_rc_cnt < UART2_BUF_SIZE)	//防止数据溢出
		{
			uart2_buf[uart2_rc_cnt++] = S2BUF;
		}
    }
}

void TM2_Isr() interrupt 12 using 1
{
    AUXINTIF &= ~T2IF;		//清中断标志
}


// ===================== 主函数 ===================== //
void main()
{
	uchar i = 0;
	uchar j = 0;
	P1M0 = 0;
	P1M1 = 0;
//	P3M0 = 0;
//	P3M1 = 0;
	Timer1Init();	//定时器1初始化
	Uart1Init();	//串口1初始化
	ES = 1;		   	//串口1中断使能
	Uart2Init();
	IE2 = 0x01;		//串口2中断使能
	ET1 = 1;		//定时器1中断使能
	EA = 1;		   	//全局中断使能

	//参数配置
	SMS310_Add = IapRead(0x0000);	//取出地址参数
	LED_nACK1 = 0;

	while (1)
	{
	 	//串口1数据处理 MAXBUS------------------------
		if(uart1_rc_ok == 1)
		{
			Delay10ms(1);
			//生成应答数据包并发送
			Uart1Send(uart1_buf, SMS310Processing(uart1_buf, uart1_rc_cnt));
			uart1_rc_cnt = 0;
			uart1_rc_interval = 0;
			uart1_rc_ok = 0;
		}

		


		//串口2短信模块相关指令处理------------------------
		
		if(work_interval == 0)
		{
			if(uart2_wait_ack == 0)	//如果收到了应答，就查信号质量
			{
				LED_nACK2 = 0;	   	//收到应答指示
				work_interval = 10;	//设定工作间隔时间
				usr_get_csq();		// 查询DTU模块信号质量

				uart2_wait_ack = 1;
				LED_nACK2 = 1;		//熄灯
			}
			else
			{
//				LED_nACK2 = 1;
				work_interval = 10; 	//设定工作间隔时间
//				usr_send_sms("18273061666", 3, "我爱我的祖国", 12);
				usr_get_csq();

				uart2_wait_ack = 1;
				LED_nACK2 = 1;		//熄灯
			}
		}

		//串口2接收数据处理
		if(uart2_rc_ok == 1)
		{
			usr_processing(uart2_buf, uart2_rc_cnt);

			uart2_rc_cnt = 0;
			uart2_rc_interval = 0;
			uart2_rc_ok = 0;

			uart2_wait_ack = 0;
		}
	}
}