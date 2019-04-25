#ifndef __UART1_H_
#define __UART1_H_


// ====================== 宏定义 ====================== //
#ifndef FOSC
	#define	FOSC 11059200UL	//IRC频率
#endif
#define BAUD1 9600UL		//串口1波特率
#define	BRT1 (65536 - FOSC / BAUD1 / 4) 	//串口1波特率控制
#define UART1_BUF_SIZE 160	//串口1缓存大小


extern bit uart1_busy;		//串口1发送忙标志
extern bit uart1_rc_ok;		//表示接收1帧完成
extern bit uart1_wait_ack;	//串口1等待应答标志
extern unsigned char uart1_rc_cnt;	//串口1接收计数
extern unsigned char uart1_rc_interval;	//串口1数据接收间隔
extern unsigned char xdata uart1_buf[UART1_BUF_SIZE];


void Uart1Init();
void Uart1Send(unsigned char* ptr, unsigned char len);

#endif /* __UART1_H_ */