// ==================== ����ͷ�ļ� ==================== //
#include "stc8f.h"
#include "sms310.h"	//����ģ��
#include "iap.h"	//EEPROM����    
#include "uart1.h"	//����1
#include "uart2.h"	//����2
#include "usr_gprs_730.h"	//USR_GPRS_730 DTUģ��
#include "pcb_rs485convertor180621.h" //ת���嶨��


// ====================== �궨�� ====================== //
//#define	FOSC	11059200UL	//IRCƵ��


// ===================== ȫ�ֱ��� ===================== //
uint t1_cnt = 0;		//��ʱ��1�ü���
uchar work_interval = 0;//while �������
uchar get_result = 0;	//�õ����
uchar wait_delay = 0;	//�ȴ�Ӧ����ʱ



// ====================== �Ӻ��� ====================== //
// �����ʱ����λ10ms@11.0592MHz
void Delay10ms(unsigned char delay)	
{
	unsigned char i, j;
	do
	{
		i = 108;
		j = 145;
		do
		{
			while (--j);
		} while (--i);
	} while (--delay);
}

// ��ʱ��1��������	1����@11.0592MHz ���д���1��ʼ��ʱ�����ֲ����ѱ�����
void Timer1Init(void)		
{
//	AUXR |= T1x12;	//��ʱ��ʱ��1Tģʽ
//	TMOD &= 0x0F;	//���ö�ʱ��ģʽ
//	TL1 = 0xCD;		//���ö�ʱ��ֵ
//	TH1 = 0xD4;		//���ö�ʱ��ֵ
	TF1 = 0;		//���TF1��־
//	TR1 = 1;		//��ʱ��1��ʼ��ʱ
//	ET1 = 1;		//ʹ�ܶ�ʱ��1�ж�
}



// ===================== �жϷ��� ===================== //
void TM1_Isr() interrupt 3 using 1	//��ʱ��1�ж�
{
	TF1 = 0;
	//9600��������ÿ���жϼ��Լ26us
	if(t1_cnt++ > 38400)	//26us * 38400 = 1s
	{
		t1_cnt = 0;

		if(work_interval > 0)
		{
			work_interval--;
		}
	}

	//����1�ַ�����������ж�һ֡����
	//9600�������£�1��ʼλ��8����λ��1ֹͣλ��
	//ÿ����ʱ��1000000��9600=104.2us
	//ÿ�ַ�ʱ��1000000��9600��10=1041.7us
	//��3.5�����ʱ��Ϊ֡���
	//1000000��9600��10��3.5=3645.8us
	if((uart1_rc_cnt > 0) && (uart1_rc_ok == 0))
	{
		uart1_rc_interval++;
		if(uart1_rc_interval > 140)
		{
			uart1_rc_interval = 0;
			uart1_rc_ok = 1;
		}
	}

	//����2�ַ�����������ж�һ֡����
	//9600�������£�1��ʼλ��8����λ��1ֹͣλ��
	//ÿ����ʱ��1000000��9600=104.2us
	//ÿ�ַ�ʱ��1000000��9600��10=1041.7us
	//��3.5�����ʱ��Ϊ֡���
	//1000000��9600��10��3.5=3645.8us
	if((uart2_rc_cnt > 0) && (uart2_rc_ok == 0))
	{
		uart2_rc_interval++;
		if(uart2_rc_interval > 140)
		{
			uart2_rc_interval = 0;
			uart2_rc_ok = 1;
		}
	}
}

void Uart1Isr() interrupt 4 using 1	//����1�ж�
{
	if (TI)
	{
		TI = 0;
		uart1_busy = 0;
	}
	if (RI)
	{
		RI = 0;
		uart1_rc_interval = 0;
		if(uart1_rc_cnt < UART1_BUF_SIZE)	//��ֹ�������
		{
			uart1_buf[uart1_rc_cnt++] = SBUF;
		}
	}
}

void Uart2_Isr() interrupt 8 using 1
{
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;		//�巢���жϱ�־
		uart2_busy = 0;
    }
    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;     //������жϱ�־
		uart2_rc_interval = 0;
		if(uart2_rc_cnt < UART2_BUF_SIZE)	//��ֹ�������
		{
			uart2_buf[uart2_rc_cnt++] = S2BUF;
		}
    }
}

void TM2_Isr() interrupt 12 using 1
{
    AUXINTIF &= ~T2IF;		//���жϱ�־
}


// ===================== ������ ===================== //
void main()
{
	uchar i = 0;
	uchar j = 0;
	P1M0 = 0;
	P1M1 = 0;
//	P3M0 = 0;
//	P3M1 = 0;
	Timer1Init();	//��ʱ��1��ʼ��
	Uart1Init();	//����1��ʼ��
	ES = 1;		   	//����1�ж�ʹ��
	Uart2Init();
	IE2 = 0x01;		//����2�ж�ʹ��
	ET1 = 1;		//��ʱ��1�ж�ʹ��
	EA = 1;		   	//ȫ���ж�ʹ��

	//��������
	SMS310_Add = IapRead(0x0000);	//ȡ����ַ����
	LED_nACK1 = 0;

	while (1)
	{
	 	//����1���ݴ��� MAXBUS------------------------
		if(uart1_rc_ok == 1)
		{
			Delay10ms(1);
			//����Ӧ�����ݰ�������
			Uart1Send(uart1_buf, SMS310Processing(uart1_buf, uart1_rc_cnt));
			uart1_rc_cnt = 0;
			uart1_rc_interval = 0;
			uart1_rc_ok = 0;
		}

		


		//����2����ģ�����ָ���------------------------
		
		if(work_interval == 0)
		{
			if(uart2_wait_ack == 0)	//����յ���Ӧ�𣬾Ͳ��ź�����
			{
				LED_nACK2 = 0;	   	//�յ�Ӧ��ָʾ
				work_interval = 10;	//�趨�������ʱ��
				usr_get_csq();		// ��ѯDTUģ���ź�����

				uart2_wait_ack = 1;
				LED_nACK2 = 1;		//Ϩ��
			}
			else
			{
//				LED_nACK2 = 1;
				work_interval = 10; 	//�趨�������ʱ��
//				usr_send_sms("18273061666", 3, "�Ұ��ҵ����", 12);
				usr_get_csq();

				uart2_wait_ack = 1;
				LED_nACK2 = 1;		//Ϩ��
			}
		}

		//����2�������ݴ���
		if(uart2_rc_ok == 1)
		{
			usr_processing(uart2_buf, uart2_rc_cnt);

			uart2_rc_cnt = 0;
			uart2_rc_interval = 0;
			uart2_rc_ok = 0;

			uart2_wait_ack = 0;
		}
	}
}