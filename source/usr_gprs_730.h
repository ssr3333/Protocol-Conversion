/* 
 * File:   usr_gprs_730.h
 * Author: Song Songru
 *
 * Created on 2018年11月13日, 16:30
 */

#ifndef __USR_GPRS_730_H_
#define __USR_GPRS_730_H_

// 应答代码
#define USR_ACK_REBOOT		0x00	//复位成功后提示信息
#define USR_ACK_CSQ 		0x01	//查询信号质量
#define USR_ACK_SMSEND 		0x02	//发送短信指令接受
#define USR_ACK_SMSENDOK	0x04	//短信息发送成功
// 错误返回代码
#define USR_ACK_PARA_ERR 	0x03	//命令参数错误或缺少参数													
#define USR_ACK_ERROR		0x58	//无效的命令或命令格式错误
// 短信发送编码方式
#define USR_TYPE_ASCII		0x31	//ASCII编码，7bit
#define USR_TYPE_8BIT		0x32 	//8bit编码方式
#define USR_TYPE_UCS8		0x33	//中英文编码方式

// 变量定义
extern unsigned char usr_cmdbuf[20];
extern unsigned char usr_cmdlen;
extern unsigned char usr_rssi;

// 功能函数定义
void  usr_send_serial_cmd(unsigned char* usr_cmdbuf, unsigned char usr_cmdlen); 	//发送串口指令
unsigned char usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len);	//发送短信
//unsigned char usr_get_sockalk(unsigned char channel);	//获取socketA连接状态
unsigned char usr_get_csq();	//查询模块网络信号质量
unsigned char usr_processing(unsigned char* dat, unsigned char len);	//处理接收到的数据

#endif /* __USR_GPRS_730_H_ */