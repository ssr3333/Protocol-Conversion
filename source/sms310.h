/* 
 * File:   sms310.h
 * Author: Song Songru
 *
 * Created on 2018��7��5��, 09:08
 */

#ifndef __SMS310_H_
#define	__SMS310_H_


/*===================================�궨��====================================*/



/*===================================ȫ�ֱ���==================================*/
extern uchar idata phone[11];
extern uchar SMS310_Add;
extern bit SMS310_Unread;


/*===================================��������==================================*/
uchar SMS310Processing(uchar* ptr, uchar len);	//���ݰ�����
uchar SMS310MakeResponse(uchar* ptr, uchar len);//����Ӧ������
uchar CalculateSum(uchar* ptr, uchar len);	//��У��

#endif	/* __SMS310_H_ */