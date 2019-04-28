/* 
 * File:   usr_gprs_730.c
 * Author: Song Songru
 *
 * Created on 2019��04��25��, 14:30
 */
#include <string.h>
#include "stc8f.h"
#include "usr_gprs_730.h"
#include "sms310.h"
//#include "uart1.h"
#include "uart2.h"


// --------------------   �궨��   --------------------
#define USR_MSG_SMSEND 		"usr.cn#\r\nOK\r\n"	//���Ͷ���ָ����ճɹ���12�ֽ�
#define USR_MSG_SMSENDOK 	"\r\nSMSEND OK\r\n"	//���ŷ��ͳɹ�Ӧ��13�ֽ� 
#define USR_MSG_CSQ 		"usr.cn#\r\n+CSQ:"	//��ѯ�ź�����Ӧ���ǰ14�ֽ�
#define USR_MSG_RBT 		"Reboot OK"			//��λ�ɹ���ʾ��Ϣ��9�ֽ�
#define USR_MSG_FAIL		"Register Failed!\r\nModule will restart...\r\n"	//ģ��ע��ʧ�ܵ���ʾ��
#define USR_MSG_SMSRCV		"\r\n+SMSRCV:"		//�յ�����ʱ����ʾ��Ϣ��ǰ10�ֽ�

#define PCB_CMD_RST			"SWRST"				//ת������λָ��
#define PASSWORD "usr.cn"	//����ATָ������


// --------------------  �������� --------------------
extern unsigned char work_interval;	//����main.c ��Ĺ����������

unsigned char usr_cmdbuf[20] = 		//ָ�����
	{
		'u', 's', 'r', '.', 'c', 'n', '#', 'A', 'T', '+',
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
	};
unsigned char usr_cmdlen = 0;		//�������ݳ���
unsigned char usr_rssi = 0;			//�ź����� ��һ��20������������ֵ31��
unsigned char usr_sim_status = 1;	//SIM��״̬
unsigned char usr_sms_result = 1;	//���Ͷ��Ž��
unsigned char usr_work_status = 1;  //����������


// --------------------  ���ܺ���  --------------------
/// <summary>
/// ͨ�����ڷ���ָ��
/// </summary>
/// <param name="text">ָ������</param>
/// <param name="len">ָ��Bytes����</param>
void usr_send_serial_cmd(unsigned char* cmd, unsigned char len)
{
	Uart2Send(cmd, len);
}


/// <summary>
/// ���Ͷ���Ϣ
/// </summary>
/// <param name="dstnum">Ŀ��绰����</param>
/// <param name="type">���뷽ʽ 1-ASCII������ѹ��,2-8bits������ѹ��,3-UCS8��Ӣ�ķ�ʽ</param>
/// <param name="text">��������</param>
/// <param name="len">����Bytes����(ASCIIģʽ���160�ֽ�,8bitģʽ140�ֽ�,UCS8ģʽ70�ֽ�)</param>
/// <returns>����ָ�����ݳ���</returns>
void usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len)
{
	//ָ��Ƚϳ���ʹ��UART2�Ļ��������ȸ���֡ͷ����
	memcpy(uart2_buf, usr_cmdbuf, 10);
	usr_cmdlen = 10;	//֡ͷ����
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
	
	usr_send_serial_cmd(uart2_buf, usr_cmdlen);	//ͨ�����ڷ���ָ��
	usr_sms_result = 0;		//Ԥ�÷���ʧ�ܱ�־λ  	
}


/// <summary>
/// ��ѯSocketͨ���Ƿ��ѽ�������
/// </summary>
/// <param name="channel">Socketͨ��(A/B/C/D)</param>
/// <returns>����ָ�����ݳ���</returns>
//unsigned char usr_get_sockalk(unsigned char channel)
//{
//	memcpy(usr_cmdbuf, "AT+SOCKALK\r\n", 12);
//	switch(channel)	//�ж�Ҫ��ѯ��Socket
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
/// ��ѯģ��������ź�ǿ�ȣ��ź�����1��20��������,��ֵ31��
/// </summary>
/// <returns>����ָ�����ݳ���</returns>
void usr_get_csq()
{
	usr_send_serial_cmd("usr.cn#AT+CSQ\r", 14);
}


/// <summary>
/// AT��Ӧ���ݴ���
/// </summary>
/// <param name="channel">Socketͨ��(A/B/C/D)</param>
/// <returns>Ӧ��ָ�����</returns>
unsigned char usr_processing(unsigned char* dat, unsigned char len)
{
	if(strncmp(dat, USR_MSG_CSQ, 14) == 0)
	{
		//��ѯ�ź������յ�Ӧ��
		if(dat[17] == ',') 	//ͨ������λ���ж�rssi�����2λ������1λ��
		{
			//����е�������ASCII���룬Ҫ����0x30������ֵ
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
		//���Ͷ��ŵ�ָ��Ӧ��
		usr_sim_status = 1;	//�ܷ�������˵��SIM������
		if((sms310_wait_send > 0) && (work_interval > 1))	//����ж��Ŵ��������ҵȴ�������1��
		{
			work_interval = 0;	//���Ͻ�����һ�ַ��Ͷ��Ź���
		}
		return USR_MSG_SMSEND;
	}
	else if(strncmp(dat, USR_MSG_SMSENDOK, 13) == 0)
	{
		//���ŷ��ͳɹ��Ļ�ִ��Ϣ
		usr_sms_result = 1;	//��λ���ŷ��ͳɹ���־
		return USR_MSG_SMSENDOK;
	}
	else if(strncmp(dat, USR_MSG_RBT, 9) == 0)//ǰ���ֿ����и�λ�ڼ������
	{
		//ģ���������
		usr_work_status = 1;
		return USR_MSG_RBT;
	}
	else if(dat[0] == 0x00) //ǰ���ǿյģ�ȥ��
	{
		//Uart1Send("unknow\r\n", 8);
		usr_processing(dat + 1, len -1);
	}
	else if(strncmp(dat, PCB_CMD_RST, 5) == 0)
	{
		IAP_CONTR = SWRST;	//��λ
	}
	else if(strncmp(dat, USR_MSG_SMSRCV, 10) == 0)
	{
		sms310_unread++;	//��־��δ������
		memcpy(phone[0], dat + 13, 11);	//�����绰����
		memcpy(sms_buf, dat + 46, len - 46 - 3);	//������Ϣ���ݣ�ȥ��ǰ��46���̶��ֽڣ�ȥ��ĩβ��\r\n
	}
	else if(strncmp(dat, USR_MSG_FAIL, 42) == 0)
	{
		//����ע��ʧ��
		usr_sim_status = 0;	//������������SIM��δ��������
	}

	return 0xFF;
}