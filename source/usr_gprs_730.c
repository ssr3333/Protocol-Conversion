/* 
 * File:   usr_gprs_730.c
 * Author: Song Songru
 *
 * Created on 2019��04��25��, 14:30
 */
#include <string.h>
#include "stc8f.h"
#include "usr_gprs_730.h"
#include "uart1.h"
#include "uart2.h"


#define usr_ack_smsend 		"usr.cn#\r\nOK\r\n"	//���Ͷ���ָ����ճɹ���12�ֽ�
#define usr_ack_smsendok 	"\r\nSMSEND OK\r\n"	//���ŷ��ͳɹ�Ӧ��13�ֽ� 
#define usr_ack_csq 		"usr.cn#\r\n+CSQ:"	//��ѯ�ź�����Ӧ���ǰ14�ֽ�
#define usr_ack_reboot 		"Reboot OK"			//��λ�ɹ���ʾ��Ϣ��9�ֽ�

#define pcb_cmd_rst			"SWRST"				//ת������λָ��


unsigned char usr_cmdbuf[20] = 		//ָ�����
	{
		'u', 's', 'r', '.', 'c', 'n', '#', 'A', 'T', '+',
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
	};
unsigned char usr_cmdlen = 0;		//�������ݳ���
unsigned char usr_rssi = 0;			//�ź����� ��һ��20������������ֵ31��

#define PASSWORD "usr.cn"	//����ATָ������
#define PWDLEN 6   			//���볤��


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
unsigned char usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len)
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
	
	usr_send_serial_cmd(uart2_buf, usr_cmdlen);  	

	return usr_cmdlen;
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
/// AT��Ӧ���ݴ���
/// </summary>
/// <param name="channel">Socketͨ��(A/B/C/D)</param>
/// <returns>Ӧ��ָ�����</returns>
unsigned char usr_processing(unsigned char* dat, unsigned char len)
{
	if(strncmp(dat, usr_ack_csq, 14) == 0)
	{
		//��ѯ�ź������յ�Ӧ��
		if(dat[16] == ',') 	//ͨ������λ���ж�rssi�����2λ������1λ��
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
		//���Ͷ��ŵ�ָ��Ӧ��
		Uart1Send("smsend\r\n", 8);
		return USR_ACK_SMSEND;
	}
	else if(strncmp(dat, usr_ack_smsendok, 13) == 0)
	{
		//���ŷ��ͳɹ��Ļ�ִ��Ϣ
		Uart1Send("sms ok\r\n", 8);
		return USR_ACK_SMSENDOK;
	}
	else if(strncmp(dat, usr_ack_reboot, 9) == 0)//ǰ���ֿ����и�λ�ڼ������
	{
		//ģ���������
		Uart1Send("reboot ok\r\n", 11);
		return USR_ACK_REBOOT;
	}
	else if(dat[0] == 0x00) //ǰ���ǿյģ�ȥ��
	{
		//Uart1Send("unknow\r\n", 8);
		usr_processing(dat + 1, len -1);
	}
	else if(strncmp(dat, pcb_cmd_rst, 5) == 0)
	{
		IAP_CONTR = SWRST;	//��λ
	}

	return 0xFF;
}