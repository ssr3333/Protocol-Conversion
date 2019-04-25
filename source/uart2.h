#ifndef __UART2_H_
#define __UART2_H_


// ====================== 宏定义 ====================== //
#ifndef FOSC
	#define	FOSC 11059200UL	//IRC频率
#endif

#define BAUD2 9600UL		//串口2波特率
#define	BRT2 (65536 - FOSC / BAUD2 / 4) 	//串口2波特率控制
#define UART2_BUF_SIZE 160	//串口2缓存大小


extern bit uart2_busy;		//串口2发送忙标志
extern bit uart2_rc_ok;		//表示接收1帧完成
extern bit uart2_wait_ack;	//串口2等待应答标志
extern unsigned char uart2_rc_cnt;	//串口2接收计数
extern unsigned char uart2_rc_interval;	//串口2数据接收间隔
extern unsigned char xdata uart2_buf[UART2_BUF_SIZE];


void Uart2Init();
void Uart2Send(unsigned char* ptr, unsigned char len);

#endif /* __UART2_H_ */