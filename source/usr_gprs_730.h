/* 
 * File:   usr_gprs_730.h
 * Author: Song Songru
 *
 * Created on 2018��11��13��, 16:30
 */

#ifndef __USR_GPRS_730_H_
#define __USR_GPRS_730_H_

// Ӧ�����
#define USR_ACK_REBOOT		0x00	//��λ�ɹ�����ʾ��Ϣ
#define USR_ACK_CSQ 		0x01	//��ѯ�ź�����
#define USR_ACK_SMSEND 		0x02	//���Ͷ���ָ�����
#define USR_ACK_SMSENDOK	0x04	//����Ϣ���ͳɹ�
// ���󷵻ش���
#define USR_ACK_PARA_ERR 	0x03	//������������ȱ�ٲ���													
#define USR_ACK_ERROR		0x58	//��Ч������������ʽ����
// ���ŷ��ͱ��뷽ʽ
#define USR_TYPE_ASCII		0x31	//ASCII���룬7bit
#define USR_TYPE_8BIT		0x32 	//8bit���뷽ʽ
#define USR_TYPE_UCS8		0x33	//��Ӣ�ı��뷽ʽ

// ��������
extern unsigned char usr_cmdbuf[20];
extern unsigned char usr_cmdlen;
extern unsigned char usr_rssi;

// ���ܺ�������
void  usr_send_serial_cmd(unsigned char* usr_cmdbuf, unsigned char usr_cmdlen); 	//���ʹ���ָ��
unsigned char usr_send_sms(unsigned char* dstnum, unsigned char type, unsigned char* text, unsigned char len);	//���Ͷ���
//unsigned char usr_get_sockalk(unsigned char channel);	//��ȡsocketA����״̬
unsigned char usr_get_csq();	//��ѯģ�������ź�����
unsigned char usr_processing(unsigned char* dat, unsigned char len);	//������յ�������

#endif /* __USR_GPRS_730_H_ */