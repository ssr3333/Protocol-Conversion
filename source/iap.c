#include "stc8f.h"
#include "iap.h"
#include "intrins.h"

//测试工作频率为11.0592MHz

/* =============================================================================
 * 名  称：关闭IAP功能
 * 输  入：
 * 输  出：
 * 功  能：EEPROM锁定
 * ===========================================================================*/
void IapIdle()
{
    IAP_CONTR = 0;                              //关闭IAP功能
    IAP_CMD = 0;                                //清除命令寄存器
    IAP_TRIG = 0;                               //清除触发寄存器
    IAP_ADDRH = 0x80;                           //将地址设置到非IAP区域
    IAP_ADDRL = 0;
}

/* =============================================================================
 * 名  称：IAP读
 * 输  入：要读取的寄存器ADD
 * 输  出：寄存器内容
 * 功  能：读取指定ADD的EEPROM数据
 * ===========================================================================*/
unsigned char IapRead(int addr)
{
    char dat;

    IAP_CONTR = WT_12M;                         //使能IAP
    IAP_CMD = 1;                                //设置IAP读命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    dat = IAP_DATA;                             //读IAP数据
    IapIdle();                                  //关闭IAP功能

    return dat;
}

/* =============================================================================
 * 名  称：IAP写
 * 输  入：要写的寄存器ADD
 * 输  出：数据内容
 * 功  能：向指定ADD的寄存器写入数据
 * ===========================================================================*/
void IapWrite(int addr, unsigned char dat)
{
    IAP_CONTR = WT_12M;                         //使能IAP
    IAP_CMD = 2;                                //设置IAP写命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_DATA = dat;                             //写IAP数据
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    IapIdle();                                  //关闭IAP功能
}

/* =============================================================================
 * 名  称：IAP擦除
 * 输  入：要擦除的寄存器ADD
 * 输  出：
 * 功  能：擦除指定ADD的EEPROM数据，擦除后数据为0xFF
 * ===========================================================================*/
void IapErase(int addr)
{
    IAP_CONTR = WT_12M;                         //使能IAP
    IAP_CMD = 3;                                //设置IAP擦除命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();                                    //
    IapIdle();                                  //关闭IAP功能
}