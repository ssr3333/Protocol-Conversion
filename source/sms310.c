/* 
 * File:   sms310.c
 * Author: Song Songru
 *
 * Created on 2018年07月05日, 16:21
 */
#include "stc8f.h"
#include "sms310.h"
#include "iap.h"
#include "uart2.h"
#include "usr_gprs_730.h"


// --------------------  变量定义  --------------------
unsigned char idata sms_buf[140];	//短信缓存
unsigned char sms_len = 0;			//短信长度
unsigned char xdata phone[5][11];	//电话号码
unsigned char sms310_add = 3; 		//模块地址初始值
unsigned char sms310_unread = 0;		//有未读短信标志
unsigned char sms310_wait_send = 0;	//待发短信条数


/* =============================================================================
 * 名  称：数据处理
 * 输  入：
 * 输  出：应答数据byte长度
 * 功  能：
 * ===========================================================================*/
unsigned char SMS310Processing(unsigned char* ptr, unsigned char len)
{
	unsigned char ret_len = 0;	//应答数据长度
	unsigned char i, j;	//循环用变量

	//检查固定包头
	if(ptr[0] == 0xAF && ptr[1] == 0x4C && ptr[2] == 0xED && ptr[3] == 0xF8)
	{
		//检查地址是否匹配，或者是改地址的指令应答
		if((ptr[4] == sms310_add) || (ptr[5] == 0x04))
		{
			//检查校验和
			if(CalculateSum(ptr, len) == 0x00)
			{
				//检查功能码
				switch(ptr[5])
				{
					case 0x01: 	//查询版本号
						if(ptr[6] == 0x00 && ptr[7] == 0x00 && ptr[8] == 0x00)
						{
							//生成应答数据包
							ptr[5] = 0x02;	//回复版本号的功能码
							ptr[6] = 0x00;
							ptr[7] = 0x0B;	//回复参数字节数
							ptr[8] = 0x00;
							ptr[9] = 0x00;
							ptr[10] = 0x00;
							ptr[11] = 0x0A;
							ptr[12] = 0x00;	//版本号第4节
							ptr[13] = 0x06;	//版本号第3节
							ptr[14] = 0x00;	//版本号第2节
							ptr[15] = 0x02;	//版本号第1节
							ptr[16] = 0x21;
							ptr[17] = 0x04;
							ptr[18] = 0x00;
							ptr[19] = 0x00;
							
							return SMS310MakeResponse(ptr, 20);
						}
						break;
					case 0x02:	//查询版本号的应答
						{
							return 240 + 2;
						}
						break;
					case 0x03:	//设置地址
						if(ptr[6] == 0x00 && ptr[7] == 0x01 && ptr[8] == 0x00)
						{
							sms310_add = ptr[9];//保存新地址
							IapWrite(0x0000, sms310_add);//更新EEPROM
							
							//生成应答数据包
							ptr[5] = 0x04;	//设置地址响应码
							ptr[6] = 0x00;
							ptr[7] = 0x02;	//回复参数字节数
							ptr[8] = 0x00;
							ptr[9] = IapRead(0x0000);	//响应新的地址
							ptr[10] = 0x00;
							
							return SMS310MakeResponse(ptr, 11);
						}
						break;
					case 0x04:	//设置地址得到了回复
						if(ptr[4] == 0x03)	//地址
						{
							sms310_add = ptr[4];
							return 240 + 4;
						}
						break;
					case 0x05:	//发送短信指令
						if(ptr[6] == 0x00)
						{
							//指令处理
							//ptr[7-8]数据字节数
							//ptr[9-10]  0100 目标手机个数
							//ptr[11 至 10 + ptr[9] * 11] 目标手机号码，11bytes, ASCII码
							if(ptr[9] > 1)	//如果有多条短信
							{
								sms310_wait_send += ptr[9];	//获取目标手机数量
								for(j = 0; j < ptr[9]; j++)
								{
									for(i = 0; i < 11; i++)	//保存目标手机号
									{
										phone[j][i] = ptr[11 + (11 * j)+ i];
									}
								}
							}
							else
							{
								sms310_wait_send++;
								for(i = 0; i < 11; i++)
								{
									phone[sms310_wait_send - 1][i] = ptr[11 + i];
								}
							}
							//ptr[22-23] 短信内容字节数
							//ptr[24-(len-3) 短信内容
							sms_len = ptr[11 + (11 * ptr[9])];
							for(i = 0; i < sms_len; i++)//保存短信内容
							{
								sms_buf[i] = ptr[13 + (11 * ptr[9]) + i];
							}

							//生成应答数据包
							ptr[5] = 0x06;	//发送短信响应码
							ptr[6] = 0x00;
							ptr[7] = 0x02; 	//参数字节数
							ptr[8] = 0x00;
							ptr[9] = 0x00;	//参数部分
							ptr[10] = 0x00;

							return SMS310MakeResponse(ptr, 11);
						}
						break;
					case 0x07:	//查询模块状态
						if(ptr[6] == 0x00 && ptr[7] == 0x00 && ptr[8] == 0x00)
						{
							ptr[5] = 0x08;	//查询状态响应码
							ptr[6] = 0x00;
							if(sms310_unread == 0)	//所有短信均已读
							{
								ptr[7] = 0x07;	//参数字节数
								ptr[8] = 0x00;
								ptr[9] = 0x01;	//短信全部已读
								ptr[10] = 0x00;	//
								ptr[11] = 0x01;	//核心状态
//								ptr[12] = 0x01;	//SIM卡状态
								ptr[12] = usr_sim_status;
//								ptr[13] = 0x01;	//模块工作状态
								ptr[13] = usr_work_status;
//								ptr[14] = 0x00;	//短信发送结果
								ptr[14] = usr_sms_result;
								ptr[15] = (usr_rssi > 10? 0x01 : 0x00);	//GSM信号状态

								return SMS310MakeResponse(ptr, 16);
							}
							else	//有未读短信
							{
								ptr[7] = 0x1B;	//参数字节数
								ptr[8] = 0x00;
								ptr[9] = 0x00;	//短信未读
								ptr[10] = 0x00;	//
								ptr[11] = 0x01;	//核心状态
//								ptr[12] = 0x01;	//SIM卡状态
								ptr[12] = usr_sim_status;
//								ptr[13] = 0x01;	//模块工作状态
								ptr[13] = usr_work_status;
//								ptr[14] = 0x00;	//短信发送结果
								ptr[14] = usr_sms_result;
								ptr[15] = (usr_rssi > 10? 0x01 : 0x00);	//GSM信号状态
								ptr[16] = 0x02;	//短信序号
								ptr[17] = 0x00;
								ptr[18] = 0x31;	//来源号码
								ptr[19] = 0x38;
								ptr[20] = 0x32;
								ptr[21] = 0x37;
								ptr[22] = 0x33;
								ptr[23] = 0x30;
								ptr[24] = 0x36;
								ptr[25] = 0x31;
								ptr[26] = 0x36;
								ptr[27] = 0x36;
								ptr[28] = 0x36;
								ptr[29] = 0x05;	//短信内容字节数
								ptr[30] = 0x00;
								ptr[31] = 0x68;
								ptr[32] = 0x65;
								ptr[33] = 0x6C;
								ptr[34] = 0x6C;
								ptr[35] = 0x6F;
								
								sms310_unread--;	//读一条清一条

								return SMS310MakeResponse(ptr, 36);
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	return 0;
}



/* =============================================================================
 * 名  称：生成应答数据包
 * 输  入：要处理的数组指针
 * 输  出：数据byte长度
 * 功  能：
 * ===========================================================================*/
unsigned char SMS310MakeResponse(unsigned char* ptr, unsigned char len)
{
	unsigned char i = 0;
	uint sum = 0;
	for(i = 0; i < len; i++)
	{
		sum += ptr[i];
	}
	ptr[len] = sum;
	ptr[len + 1] = (sum >> 8);
	return len + 2;
}


/* =============================================================================
 * 名  称：获取短信模块版本号
 * 输  入：数据缓冲区指针，设备地址
 * 输  出：数据byte长度
 * 功  能：
 * ===========================================================================*/
//unsigned char SMS310GetVersion(unsigned char* ptr, unsigned char add)
//{
//	unsigned char i = 0;
//	uint sum = 0;
//
// 	ptr[0] = 0xAF;		//数据包头
//	ptr[1] = 0x4C;
//	ptr[2] = 0xED;
//	ptr[3] = 0xF8;
//	ptr[4] = add;   	//从设备地址
//	ptr[5] = 0x01;		//查询版本号指令码低位
//	ptr[6] = 0x00;		//查询版本号指令码高位
//	ptr[7] = 0x00;		//参数字节数低位
//	ptr[8] = 0x00;		//参数字节数高位
//	
//	for(i = 0; i < 9; i++)
//	{
//		sum += ptr[i];
//	}
//	ptr[9] = sum;			//校验和低位
//	ptr[10] = (sum >> 8);	//校验和高位
//	
//	return 11;	
//} 

/* =============================================================================
 * 名  称：生成设置地址指令数据包
 * 输  入：oldadd-旧地址, newadd-新地址
 * 输  出：数据包长度
 * 功  能：
 * ===========================================================================*/

/* =============================================================================
 * 名  称：设置模块地址
 * 输  入：数据缓冲区指针，设备原地址，设备新地址
 * 输  出：数据byte长度
 * 功  能：
 * ===========================================================================*/
//unsigned char SMS310SetAdd(unsigned char* ptr, unsigned char oldadd, unsigned char newadd)
//{
//	unsigned char i = 0;
//	uint sum = 0;
//
//	ptr[0] = 0xAF;		//数据包头
//	ptr[1] = 0x4C;
//	ptr[2] = 0xED;
//	ptr[3] = 0xF8;
//	ptr[4] = oldadd;   	//从设备地址-原地址
//	ptr[5] = 0x03;		//修改地址指令码低位
//	ptr[6] = 0x00;		//修改地址指令码高位
//	ptr[7] = 0x01;		//参数字节数低位
//	ptr[8] = 0x00;		//参数字节数高位
//	ptr[9] = newadd;	//参数-新地址
//	 
//	for(i = 0; i < 10; i++)
//	{
//		sum += ptr[i];
//	}
//	ptr[10] = sum;
//	ptr[11] = (sum >> 8);
//
//	return 12;
//}



/* =============================================================================
 * 名  称：数据和校验
 * 输  入：ptr-数据包指针, len-数据包byte长度
 * 输  出：0-校验和正确，0xFF-校验和失败
 * 功  能：
 * ===========================================================================*/
unsigned char CalculateSum(unsigned char* ptr, unsigned char len)
{
	unsigned char i = 0;
	uint count = 0;
	//计算数据包中 除校验和部分 的和
	for(i = 0; i < (len - 2); i++)
	{
		count += ptr[i];
	}
	//返回校验和的结果
	if(count - (ptr[len - 2] + (ptr[len - 1] << 8)) == 0)
	{
		return 0;
	}
	else
	{
		return 0xFF;
	}		
}

/* =============================================================================
 * 名  称：发送短信
 * 输  入：dst_phone目标手机号码, dat-短信数据,  dat_len-数据字节数
 * 输  出：
 * 功  能：
 * ===========================================================================*/
//void SendSMS(unsigned char* dst_phone, unsigned char* dat, unsigned char dat_len)
//{
//	Uart2Send("SMS", 3);
//	Uart2Send(dst_phone, 11);
//	Uart2Send("#", 1);
//	Uart2Send(dat, dat_len);
//}