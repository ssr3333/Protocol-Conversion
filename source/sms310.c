/* 
 * File:   sms310.c
 * Author: Song Songru
 *
 * Created on 2018��07��05��, 16:21
 */
#include "stc8f.h"
#include "sms310.h"
#include "iap.h"
#include "uart2.h"
#include "usr_gprs_730.h"


// --------------------  ��������  --------------------
unsigned char idata sms_buf[140];	//���Ż���
unsigned char sms_len = 0;			//���ų���
unsigned char xdata phone[5][11];	//�绰����
unsigned char sms310_add = 3; 		//ģ���ַ��ʼֵ
unsigned char sms310_unread = 0;		//��δ�����ű�־
unsigned char sms310_wait_send = 0;	//������������


/* =============================================================================
 * ��  �ƣ����ݴ���
 * ��  �룺
 * ��  ����Ӧ������byte����
 * ��  �ܣ�
 * ===========================================================================*/
unsigned char SMS310Processing(unsigned char* ptr, unsigned char len)
{
	unsigned char ret_len = 0;	//Ӧ�����ݳ���
	unsigned char i, j;	//ѭ���ñ���

	//���̶���ͷ
	if(ptr[0] == 0xAF && ptr[1] == 0x4C && ptr[2] == 0xED && ptr[3] == 0xF8)
	{
		//����ַ�Ƿ�ƥ�䣬�����Ǹĵ�ַ��ָ��Ӧ��
		if((ptr[4] == sms310_add) || (ptr[5] == 0x04))
		{
			//���У���
			if(CalculateSum(ptr, len) == 0x00)
			{
				//��鹦����
				switch(ptr[5])
				{
					case 0x01: 	//��ѯ�汾��
						if(ptr[6] == 0x00 && ptr[7] == 0x00 && ptr[8] == 0x00)
						{
							//����Ӧ�����ݰ�
							ptr[5] = 0x02;	//�ظ��汾�ŵĹ�����
							ptr[6] = 0x00;
							ptr[7] = 0x0B;	//�ظ������ֽ���
							ptr[8] = 0x00;
							ptr[9] = 0x00;
							ptr[10] = 0x00;
							ptr[11] = 0x0A;
							ptr[12] = 0x00;	//�汾�ŵ�4��
							ptr[13] = 0x06;	//�汾�ŵ�3��
							ptr[14] = 0x00;	//�汾�ŵ�2��
							ptr[15] = 0x02;	//�汾�ŵ�1��
							ptr[16] = 0x21;
							ptr[17] = 0x04;
							ptr[18] = 0x00;
							ptr[19] = 0x00;
							
							return SMS310MakeResponse(ptr, 20);
						}
						break;
					case 0x02:	//��ѯ�汾�ŵ�Ӧ��
						{
							return 240 + 2;
						}
						break;
					case 0x03:	//���õ�ַ
						if(ptr[6] == 0x00 && ptr[7] == 0x01 && ptr[8] == 0x00)
						{
							sms310_add = ptr[9];//�����µ�ַ
							IapWrite(0x0000, sms310_add);//����EEPROM
							
							//����Ӧ�����ݰ�
							ptr[5] = 0x04;	//���õ�ַ��Ӧ��
							ptr[6] = 0x00;
							ptr[7] = 0x02;	//�ظ������ֽ���
							ptr[8] = 0x00;
							ptr[9] = IapRead(0x0000);	//��Ӧ�µĵ�ַ
							ptr[10] = 0x00;
							
							return SMS310MakeResponse(ptr, 11);
						}
						break;
					case 0x04:	//���õ�ַ�õ��˻ظ�
						if(ptr[4] == 0x03)	//��ַ
						{
							sms310_add = ptr[4];
							return 240 + 4;
						}
						break;
					case 0x05:	//���Ͷ���ָ��
						if(ptr[6] == 0x00)
						{
							//ָ���
							//ptr[7-8]�����ֽ���
							//ptr[9-10]  0100 Ŀ���ֻ�����
							//ptr[11 �� 10 + ptr[9] * 11] Ŀ���ֻ����룬11bytes, ASCII��
							if(ptr[9] > 1)	//����ж�������
							{
								sms310_wait_send += ptr[9];	//��ȡĿ���ֻ�����
								for(j = 0; j < ptr[9]; j++)
								{
									for(i = 0; i < 11; i++)	//����Ŀ���ֻ���
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
							//ptr[22-23] ���������ֽ���
							//ptr[24-(len-3) ��������
							sms_len = ptr[11 + (11 * ptr[9])];
							for(i = 0; i < sms_len; i++)//�����������
							{
								sms_buf[i] = ptr[13 + (11 * ptr[9]) + i];
							}

							//����Ӧ�����ݰ�
							ptr[5] = 0x06;	//���Ͷ�����Ӧ��
							ptr[6] = 0x00;
							ptr[7] = 0x02; 	//�����ֽ���
							ptr[8] = 0x00;
							ptr[9] = 0x00;	//��������
							ptr[10] = 0x00;

							return SMS310MakeResponse(ptr, 11);
						}
						break;
					case 0x07:	//��ѯģ��״̬
						if(ptr[6] == 0x00 && ptr[7] == 0x00 && ptr[8] == 0x00)
						{
							ptr[5] = 0x08;	//��ѯ״̬��Ӧ��
							ptr[6] = 0x00;
							if(sms310_unread == 0)	//���ж��ž��Ѷ�
							{
								ptr[7] = 0x07;	//�����ֽ���
								ptr[8] = 0x00;
								ptr[9] = 0x01;	//����ȫ���Ѷ�
								ptr[10] = 0x00;	//
								ptr[11] = 0x01;	//����״̬
//								ptr[12] = 0x01;	//SIM��״̬
								ptr[12] = usr_sim_status;
//								ptr[13] = 0x01;	//ģ�鹤��״̬
								ptr[13] = usr_work_status;
//								ptr[14] = 0x00;	//���ŷ��ͽ��
								ptr[14] = usr_sms_result;
								ptr[15] = (usr_rssi > 10? 0x01 : 0x00);	//GSM�ź�״̬

								return SMS310MakeResponse(ptr, 16);
							}
							else	//��δ������
							{
								ptr[7] = 0x1B;	//�����ֽ���
								ptr[8] = 0x00;
								ptr[9] = 0x00;	//����δ��
								ptr[10] = 0x00;	//
								ptr[11] = 0x01;	//����״̬
//								ptr[12] = 0x01;	//SIM��״̬
								ptr[12] = usr_sim_status;
//								ptr[13] = 0x01;	//ģ�鹤��״̬
								ptr[13] = usr_work_status;
//								ptr[14] = 0x00;	//���ŷ��ͽ��
								ptr[14] = usr_sms_result;
								ptr[15] = (usr_rssi > 10? 0x01 : 0x00);	//GSM�ź�״̬
								ptr[16] = 0x02;	//�������
								ptr[17] = 0x00;
								ptr[18] = 0x31;	//��Դ����
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
								ptr[29] = 0x05;	//���������ֽ���
								ptr[30] = 0x00;
								ptr[31] = 0x68;
								ptr[32] = 0x65;
								ptr[33] = 0x6C;
								ptr[34] = 0x6C;
								ptr[35] = 0x6F;
								
								sms310_unread--;	//��һ����һ��

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
 * ��  �ƣ�����Ӧ�����ݰ�
 * ��  �룺Ҫ���������ָ��
 * ��  ��������byte����
 * ��  �ܣ�
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
 * ��  �ƣ���ȡ����ģ��汾��
 * ��  �룺���ݻ�����ָ�룬�豸��ַ
 * ��  ��������byte����
 * ��  �ܣ�
 * ===========================================================================*/
//unsigned char SMS310GetVersion(unsigned char* ptr, unsigned char add)
//{
//	unsigned char i = 0;
//	uint sum = 0;
//
// 	ptr[0] = 0xAF;		//���ݰ�ͷ
//	ptr[1] = 0x4C;
//	ptr[2] = 0xED;
//	ptr[3] = 0xF8;
//	ptr[4] = add;   	//���豸��ַ
//	ptr[5] = 0x01;		//��ѯ�汾��ָ�����λ
//	ptr[6] = 0x00;		//��ѯ�汾��ָ�����λ
//	ptr[7] = 0x00;		//�����ֽ�����λ
//	ptr[8] = 0x00;		//�����ֽ�����λ
//	
//	for(i = 0; i < 9; i++)
//	{
//		sum += ptr[i];
//	}
//	ptr[9] = sum;			//У��͵�λ
//	ptr[10] = (sum >> 8);	//У��͸�λ
//	
//	return 11;	
//} 

/* =============================================================================
 * ��  �ƣ��������õ�ַָ�����ݰ�
 * ��  �룺oldadd-�ɵ�ַ, newadd-�µ�ַ
 * ��  �������ݰ�����
 * ��  �ܣ�
 * ===========================================================================*/

/* =============================================================================
 * ��  �ƣ�����ģ���ַ
 * ��  �룺���ݻ�����ָ�룬�豸ԭ��ַ���豸�µ�ַ
 * ��  ��������byte����
 * ��  �ܣ�
 * ===========================================================================*/
//unsigned char SMS310SetAdd(unsigned char* ptr, unsigned char oldadd, unsigned char newadd)
//{
//	unsigned char i = 0;
//	uint sum = 0;
//
//	ptr[0] = 0xAF;		//���ݰ�ͷ
//	ptr[1] = 0x4C;
//	ptr[2] = 0xED;
//	ptr[3] = 0xF8;
//	ptr[4] = oldadd;   	//���豸��ַ-ԭ��ַ
//	ptr[5] = 0x03;		//�޸ĵ�ַָ�����λ
//	ptr[6] = 0x00;		//�޸ĵ�ַָ�����λ
//	ptr[7] = 0x01;		//�����ֽ�����λ
//	ptr[8] = 0x00;		//�����ֽ�����λ
//	ptr[9] = newadd;	//����-�µ�ַ
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
 * ��  �ƣ����ݺ�У��
 * ��  �룺ptr-���ݰ�ָ��, len-���ݰ�byte����
 * ��  ����0-У�����ȷ��0xFF-У���ʧ��
 * ��  �ܣ�
 * ===========================================================================*/
unsigned char CalculateSum(unsigned char* ptr, unsigned char len)
{
	unsigned char i = 0;
	uint count = 0;
	//�������ݰ��� ��У��Ͳ��� �ĺ�
	for(i = 0; i < (len - 2); i++)
	{
		count += ptr[i];
	}
	//����У��͵Ľ��
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
 * ��  �ƣ����Ͷ���
 * ��  �룺dst_phoneĿ���ֻ�����, dat-��������,  dat_len-�����ֽ���
 * ��  ����
 * ��  �ܣ�
 * ===========================================================================*/
//void SendSMS(unsigned char* dst_phone, unsigned char* dat, unsigned char dat_len)
//{
//	Uart2Send("SMS", 3);
//	Uart2Send(dst_phone, 11);
//	Uart2Send("#", 1);
//	Uart2Send(dat, dat_len);
//}