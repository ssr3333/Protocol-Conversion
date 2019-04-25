/* 
 * File:   usr_gprs_730.c
 * Author: Song Songru
 *
 * Created on 2019年04月25日, 14:30
 */
#include <string.h>
#include "stc8f.h"
#include "usr_gprs_730.h"
#include "uart1.h"
#include "uart2.h"


#define usr_ack_smsend 		"usr.cn#\r\nOK\r\n"	//发送短信指令接收成功，12字节
#define usr_ack_smsendok 	"\r\nSMSEND OK\r\n"	//短信发送成功应答，13字节 
#define usr_ack_csq 		"usr.cn#\r\n+CSQ:"	//查询信号质量应答的前14字节
#define usr_ack_reboot 		"Reboot OK"			//复位成功提示信息，9字节

#define pcb_cmd_rst			"SWRST"				//转换器复位指令


unsigned char usr_cmdbuf[20] = 		//指令缓冲区
	{
		'u', 's', 'r', '.', 'c', 'n', '#', 'A', 'T', '+',
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
	};
unsigned char usr_cmdlen = 0;		//缓冲数据长度
unsigned char usr_rssi = 0;			//信号质量 （一般20以上正常，满值31）

#define PASSWORD "usr.cn"	//串口AT指令密码
#define PWDLEN 6   			//密码长度


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
unsigned char usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len)
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
	
	usr_send_serial_cmd(uart2_buf, usr_cmdlen);  	

	return usr_cmdlen;
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
unsigned char usr_get_csq()
{
//	usr_cmdbuf[10] = 'C';
//	usr_cmdbuf[11] = 'S';
//	usr_cmdbuf[12] = 'Q';
//	usr_cmdbuf[13] = '\r';	//0x0D
//	usr_cmdlen = 14;
//
//	usr_send_serial_cmd(usr_cmdbuf, usr_cmdlen);
	Uart2Send("usr.cn#AT+CSQ\r", 14);
	
	return usr_cmdlen;
}


/// <summary>
/// AT回应数据处理
/// </summary>
/// <param name="channel">Socket通道(A/B/C/D)</param>
/// <returns>应答指令代码</returns>
unsigned char usr_processing(unsigned char* dat, unsigned char len)
{
	if(strncmp(dat, usr_ack_csq, 14) == 0)
	{
		//查询信号质量收到应答
		if(dat[16] == ',') 	//通过逗号位置判断rssi结果是2位数还是1位数
		{
			usr_rssi = ((dat[14] - 30) * 10) + (dat[15] - 30);
		}
		else if(dat[15] == ',')
		{
			usr_rssi = dat[14] - 30;
		}

		return USR_ACK_CSQ;
	}
	else if((strncmp(dat, usr_ack_smsend, 12) == 0))
	{
		//发送短信的指令应答
		Uart1Send("smsend\r\n", 8);
		return USR_ACK_SMSEND;
	}
	else if(strncmp(dat, usr_ack_smsendok, 13) == 0)
	{
		//短信发送成功的回执信息
		Uart1Send("sms ok\r\n", 8);
		return USR_ACK_SMSENDOK;
	}
	else if(strncmp(dat, usr_ack_reboot, 9) == 0)//前部分可能有复位期间的乱码
	{
		//模块重启完成
		Uart1Send("reboot ok\r\n", 11);
		return USR_ACK_REBOOT;
	}
	else if(dat[0] == 0x00) //前面是空的，去掉
	{
		//Uart1Send("unknow\r\n", 8);
		usr_processing(dat + 1, len -1);
	}
	else if(strncmp(dat, pcb_cmd_rst, 5) == 0)
	{
		IAP_CONTR = SWRST;	//软复位
	}

	return 0xFF;
}