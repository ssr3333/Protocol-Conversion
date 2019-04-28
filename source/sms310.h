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
extern unsigned char idata sms_buf[140];	//���Ż���
extern unsigned char xdata phone[5][11];	//����洢��
extern unsigned char sms_len;				//���ų���
extern unsigned char sms310_add;	//����ģ���ַ
extern unsigned char sms310_unread;	//�ж���δ����־
extern unsigned char sms310_wait_send;	//�����Ͷ�������


/*===================================��������==================================*/
unsigned char SMS310Processing(unsigned char* ptr, unsigned char len);	//���ݰ�����
unsigned char SMS310MakeResponse(unsigned char* ptr, unsigned char len);//����Ӧ������
unsigned char CalculateSum(unsigned char* ptr, unsigned char len);	//��У��

#endif	/* __SMS310_H_ */