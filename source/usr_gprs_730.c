/* 
 * File:   usr_gprs_730.c
 * Author: Song Songru
 *
 * Created on 2019年04月25日, 14:30
 */
#include <string.h>
#include "stc8f.h"
#include "usr_gprs_730.h"
#include "sms310.h"
//#include "uart1.h"
#include "uart2.h"


// --------------------   宏定义   --------------------
#define USR_MSG_SMSEND 		"usr.cn#\r\nOK\r\n"	//发送短信指令接收成功，12字节
#define USR_MSG_SMSENDOK 	"\r\nSMSEND OK\r\n"	//短信发送成功应答，13字节 
#define USR_MSG_CSQ 		"usr.cn#\r\n+CSQ:"	//查询信号质量应答的前14字节
#define USR_MSG_RBT 		"Reboot OK"			//复位成功提示信息，9字节
#define USR_MSG_FAIL		"Register Failed!\r\nModule will restart...\r\n"	//模块注册失败的提示语
#define USR_MSG_SMSRCV		"\r\n+SMSRCV:"		//收到短信时的提示信息的前10字节

#define PCB_CMD_RST			"SWRST"				//转换器复位指令
#define PASSWORD "usr.cn"	//串口AT指令密码


// --------------------  变量定义 --------------------
extern unsigned char work_interval;	//引用main.c 里的工作间隔参数

unsigned char usr_cmdbuf[20] = 		//指令缓冲区
	{
		'u', 's', 'r', '.', 'c', 'n', '#', 'A', 'T', '+',
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
	};
unsigned char usr_cmdlen = 0;		//缓冲数据长度
unsigned char usr_rssi = 0;			//信号质量 （一般20以上正常，满值31）
unsigned char usr_sim_status = 1;	//SIM卡状态
unsigned char usr_sms_result = 1;	//发送短信结果
unsigned char usr_work_status = 1;  //工作不正常


// --------------------  功能函数  --------------------
/// <summary>
/// 通过串口发送指令
/// </summary>
/// <param name="text">指令内容</param>
/// <param name="len">指令Bytes长度</param>
void usr_send_serial_cmd(unsigned char* cmd, unsigned char len)
{
	Uart2Send(cmd, len);
}


/// <summary>
/// 发送短信息
/// </summary>
/// <param name="dstnum">目标电话号码</param>
/// <param name="type">编码方式 1-ASCII编码有压缩,2-8bits编码无压缩,3-UCS8中英文方式</param>
/// <param name="text">短信内容</param>
/// <param name="len">短信Bytes长度(ASCII模式最大160字节,8bit模式140字节,UCS8模式70字节)</param>
/// <returns>完整指令数据长度</returns>
void usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len)
{
	//指令比较长，使用UART2的缓冲区，先复制帧头部分
	memcpy(uart2_buf, usr_cmdbuf, 10);
	usr_cmdlen = 10;	//帧头长度
	memcpy(uart2_buf + usr_cmdlen, "SMSEND=\"", 8);
	usr_cmdlen += 8;
	memcpy(uart2_buf + usr_cmdlen, dstnum, 11);
    usr_cmdlen += 11;
	memcpy(uart2_buf + usr_cmdlen, "\",", 2);
	usr_cmdlen += 2;

	uart2_buf[usr_cmdlen] = type;
	usr_cmdlen += 1;

	memcpy(uart2_buf + usr_cmdlen, ",\"", 2);
	usr_cmdlen += 2;
	memcpy(uart2_buf + usr_cmdlen, text, len);
	usr_cmdlen += len;
	memcpy(uart2_buf + usr_cmdlen, "\"\r", 2);
	usr_cmdlen += 2; 
	
	usr_send_serial_cmd(uart2_buf, usr_cmdlen);	//通过串口发送指令
	usr_sms_result = 0;		//预置发送失败标志位  	
}


/// <summary>
/// 查询Socket通道是否已建立连接
/// </summary>
/// <param name="channel">Socket通道(A/B/C/D)</param>
/// <returns>完整指令数据长度</returns>
//unsigned char usr_get_sockalk(unsigned char channel)
//{
//	memcpy(usr_cmdbuf, "AT+SOCKALK\r\n", 12);
//	switch(channel)	//判断要查询的Socket
//	{
//		case 'A':
//		case 'a':
//		case 1:
//		{
//			usr_cmdbuf[7] = 'A';
//		}break;
//
//		case 'B':
//		case 'b':
//		case 2:
//		{
//			usr_cmdbuf[7] = 'B';
//		}break;
//
//		case 'C':
//		case 'c':
//		case 3:
//		{
//			usr_cmdbuf[7] = 'C';
//		}break;
//
//		case 'D':
//		case 'd':
//		case 4:
//		{
//			usr_cmdbuf[7] = 'D';
//		}break;
//
//		default:
//		break;
//	}
//	usr_cmdlen = 12;
//
//	return usr_cmdlen;
//}

/// <summary>
/// 查询模块的网络信号强度（信号质量1般20以上正常,满值31）
/// </summary>
/// <returns>完整指令数据长度</returns>
void usr_get_csq()
{
	usr_send_serial_cmd("usr.cn#AT+CSQ\r", 14);
}


/// <summary>
/// AT回应数据处理
/// </summary>
/// <param name="channel">Socket通道(A/B/C/D)</param>
/// <returns>应答指令代码</returns>
unsigned char usr_processing(unsigned char* dat, unsigned char len)
{
	if(strncmp(dat, USR_MSG_CSQ, 14) == 0)
	{
		//查询信号质量收到应答
		if(dat[17] == ',') 	//通过逗号位置判断rssi结果是2位数还是1位数
		{
			//结果中的数字是ASCII编码，要减掉0x30才是真值
			usr_rssi = ((dat[15] - 0x30) * 10) + (dat[16] - 0x30);
		}
		else if(dat[16] == ',')
		{
			usr_rssi = dat[15] - 0x30;
		}

		return USR_MSG_CSQ;
	}
	else if((strncmp(dat, USR_MSG_SMSEND, 12) == 0))
	{
		//发送短信的指令应答
		usr_sim_status = 1;	//能发出短信说明SIM卡正常
		if((sms310_wait_send > 0) && (work_interval > 1))	//如果有短信待发，并且等待还大于1秒
		{
			work_interval = 0;	//马上进入下一轮发送短信工作
		}
		return USR_MSG_SMSEND;
	}
	else if(strncmp(dat, USR_MSG_SMSENDOK, 13) == 0)
	{
		//短信发送成功的回执信息
		usr_sms_result = 1;	//置位短信发送成功标志
		return USR_MSG_SMSENDOK;
	}
	else if(strncmp(dat, USR_MSG_RBT, 9) == 0)//前部分可能有复位期间的乱码
	{
		//模块重启完成
		usr_work_status = 1;
		return USR_MSG_RBT;
	}
	else if(dat[0] == 0x00) //前面是空的，去掉
	{
		//Uart1Send("unknow\r\n", 8);
		usr_processing(dat + 1, len -1);
	}
	else if(strncmp(dat, PCB_CMD_RST, 5) == 0)
	{
		IAP_CONTR = SWRST;	//软复位
	}
	else if(strncmp(dat, USR_MSG_SMSRCV, 10) == 0)
	{
		sms310_unread++;	//标志有未读短信
		memcpy(phone[0], dat + 13, 11);	//拷贝电话号码
		memcpy(sms_buf, dat + 46, len - 46 - 3);	//拷贝信息内容，去掉前面46个固定字节，去掉末尾“\r\n
	}
	else if(strncmp(dat, USR_MSG_FAIL, 42) == 0)
	{
		//网络注册失败
		usr_sim_status = 0;	//基本上是由于SIM卡未插入引起
	}

	return 0xFF;
}