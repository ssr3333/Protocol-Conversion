#ifndef __WDT_H_
#define __WDT_H_



/* WDT�Ĵ�����ַ������λ����
 * sfr WDT_CONTR   =   0xc1;
 * #define WDT_FLAG    0x80
 * #define EN_WDT      0x20
 * #define CLR_WDT     0x10
 * #define IDL_WDT     0x08
*/


#define CLRWDT()	WDT_CONTR |= 0x10	//ι������

// #define WDT_INIT()	WDT_CONTR = 0x23	//���ʱ��Լ0.5��
// #define WDT_INIT()	WDT_CONTR = 0x24	//���ʱ��Լ1��
#define WDT_INIT()	WDT_CONTR = 0x25	//���ʱ��Լ2��
// #define WDT_INIT()	WDT_CONTR = 0x26	//���ʱ��Լ4��
// #define WDT_INIT()	WDT_CONTR = 0x27	//���ʱ��Լ8��

#endif /*__WDT_H_*/