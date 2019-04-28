/* 
 * File:   sms310.h
 * Author: Song Songru
 *
 * Created on 2018年7月5日, 09:08
 */

#ifndef __SMS310_H_
#define	__SMS310_H_


/*===================================宏定义====================================*/



/*===================================全局变量==================================*/
extern unsigned char idata sms_buf[140];	//短信缓存
extern unsigned char xdata phone[5][11];	//号码存储区
extern unsigned char sms_len;				//短信长度
extern unsigned char sms310_add;	//短信模块地址
extern unsigned char sms310_unread;	//有短信未读标志
extern unsigned char sms310_wait_send;	//待发送短信条数


/*===================================函数声明==================================*/
unsigned char SMS310Processing(unsigned char* ptr, unsigned char len);	//数据包处理
unsigned char SMS310MakeResponse(unsigned char* ptr, unsigned char len);//生成应答数据
unsigned char CalculateSum(unsigned char* ptr, unsigned char len);	//和校验

#endif	/* __SMS310_H_ */