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
extern uchar idata phone[11];
extern uchar SMS310_Add;
extern bit SMS310_Unread;


/*===================================函数声明==================================*/
uchar SMS310Processing(uchar* ptr, uchar len);	//数据包处理
uchar SMS310MakeResponse(uchar* ptr, uchar len);//生成应答数据
uchar CalculateSum(uchar* ptr, uchar len);	//和校验

#endif	/* __SMS310_H_ */