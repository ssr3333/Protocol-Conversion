#ifndef __WDT_H_
#define __WDT_H_



/* WDT寄存器地址及功能位定义
 * sfr WDT_CONTR   =   0xc1;
 * #define WDT_FLAG    0x80
 * #define EN_WDT      0x20
 * #define CLR_WDT     0x10
 * #define IDL_WDT     0x08
*/


#define CLRWDT()	WDT_CONTR |= 0x10	//喂狗操作

// #define WDT_INIT()	WDT_CONTR = 0x23	//溢出时间约0.5秒
// #define WDT_INIT()	WDT_CONTR = 0x24	//溢出时间约1秒
#define WDT_INIT()	WDT_CONTR = 0x25	//溢出时间约2秒
// #define WDT_INIT()	WDT_CONTR = 0x26	//溢出时间约4秒
// #define WDT_INIT()	WDT_CONTR = 0x27	//溢出时间约8秒

#endif /*__WDT_H_*/