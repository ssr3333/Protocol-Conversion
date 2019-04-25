#ifndef __IAP_H_
#define __IAP_H_

/* 1T系列寄存器地址
sfr     IAP_DATA    =   0xC2;	//数据寄存器
sfr     IAP_ADDRH   =   0xC3;	//地址高字节
sfr     IAP_ADDRL   =   0xC4;	//地址低字节
sfr     IAP_CMD     =   0xC5;	//命令模式
sfr     IAP_TRIG    =   0xC6;	//命令触发寄存器
sfr     IAP_CONTR   =   0xC7;	//控制寄存器
*/

//不同时钟频率下的IAP操作等待时长参数
//设置好后由硬件完成，不需软件延时
#define WT_30M          0x80 	//SYSCLK < 30MHz
#define WT_24M          0x81	//SYSCLK < 24MHz
#define WT_20M          0x82	//SYSCLK < 20MHz
#define WT_12M          0x83	//SYSCLK < 12MHz
#define WT_6M           0x84	//SYSCLK < 6MHz
#define WT_3M           0x85	//SYSCLK < 3MHz
#define WT_2M           0x86	//SYSCLK < 2MHz
#define WT_1M           0x87	//SYSCLK < 1MHz


void IapIdle();	//关闭IAP
char IapRead(int addr);	//IAP读
void IapWrite(int addr, char dat); //IAP写
void IapErase(int addr);//IAP擦除

#endif /*__IAP_H_*/