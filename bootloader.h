#ifndef _BOOTLOADER_H
#define _BOOTLOAER_H
#include "inc\V9811.h"
#include <INTRINS.h>
/*波特率寄存器的值*/
#define D_baud115200REG 0xF9
#define D_baud19200REG  0xD5
#define D_baud9600REG   0xAB
#define D_baud4800REG   0x55
#define D_baud2400REG   0xE4
#define D_baud1200REG   0xC7
#define D_baud600REG    0x8E
#define D_baud300REG    0x1C

// 宏
#ifndef NULL
    #define NULL            0
#endif

#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

#define D_FALSE     	0
#define D_TRUE			1
#define D_ABSOLUTE  	1
#define D_RELATIVE		0

#define RI_             0x01
#define TI_             0x02

#define D_DISINT()      (EA = 0)
#define D_CLRWDT()      do {wdt0=0xA5;wdt1=0x5A; wdt0=0xA5;wdt1=0x5A;} while(0)

#define D_RISET()       (SCON5 & RI_)
#define D_RICLR()       do {SCON5 &= (~RI_); ExInt3IE &= (~BIT3);}while(0)

#define D_TISET()       (SCON5 & TI_) 
#define D_TICLR()       do {SCON5 &= (~TI_);ExInt3IE &= (~BIT2);} while(0)

#define D_TFSET()       (TCON & BIT5)
#define D_TFCLR()       (TCON &= (~BIT5))

#define D_SBUFF         (SBUF5)

// echo代码
#define D_STOP         	0x00    // 结束升级
#define D_START         0x01    // 开始升级，此时上位机可以发送第一帧数据

#define D_NEXTFRAME     0x02    // 请求下一帧数据
#define D_NEXTSECTOR    0x03    // 请求下一扇区数据

#define D_UPDATADONE    0x04    // 升级完成
#define D_UPDATAFAIL    0x05    // 升级失败

#define D_REPEATFRAM    0x06    // 重复请求上一帧数据
#define D_REPEATSECTOR  0x07    // 重复请求上一扇区数据
#define D_BTL           0x08    // bootloader标志
#define D_EXIT          0x09    // 退出bootloader

#define D_NOP()         _nop_()

#define D_V98XX         0
#define D_SH7012        1

// 全局设置
#define D_FLAGADDR      0x0FF0                          // xdata区，用于作为升级的全局标志
#define D_MCU           D_V98XX
#define D_UPDATALEN  	16							    // 每帧的升级数据数据长度
#define D_FRAMELEN      (D_UPDATALEN+6)                 // 每帧实际数据长度()
#define D_ADRESSMODE    D_ABSOLUTE					    // 寻址模式
#define D_READCHECK		D_FALSE						    // 读出比对
#define D_TIMETH        (200)                            // *50ms

// 关键标识数组下标
#define D_FRAMESTARTLC  0								// 帧头
#define D_FRAMESTOPLC	(D_FRAMELEN-1)					// 帧尾
#define D_SECTORLC		1								// 扇区号
#define D_LINELC		2								// 行号
#define D_FLAGLC		3								// 标志位(用于升级控制)
#define D_DATESTARTLC	4								// 数据开始
#define D_CHKLC			(D_FRAMELEN-2)					// 校验位

// 升级控制标识
#define D_UPDATE 		0x00
#define D_SECTORDONE	0x01
#define D_ALLDATEDONE	0x02
#define D_STOPUPDATE	0x03
#define D_STARTUPDATE   0x04
#define D_EXITBTL       0x05
// note* bootloader位于flash中的最后10k_(10或20页)(0xD800-0xFFFF),其中0xFE00页用于保存用户程序的入口地址
#if (D_MCU == D_V98XX)
    #define D_SECTORSUM		0x6c						// 128-20页
    #define D_SECTORSIZE	0x200 						// 每页大小，V98xx是512个字节 中颖7012是1024个字节
#endif
#if (D_MCU == D_SH7012)
    #define D_SECTORSUM		0x36 						// 64-10页
    #define D_SECTORSIZE/	0x400 						// 每页大小，V98xx是512个字节 中颖7012是1024个字节
#endif

#define D_FRAMENUM		(D_SECTORSIZE/D_UPDATALEN)	    // 传输一页数据需要的总帧数

//// 用户主函数入口地址
//typedef void (code*USERAPP)(void);                      // 函数指针
//#define D_MAINBASEADDR     (0x0600)

// 函数申明


extern unsigned char BTL_checkStatus(void );
extern void BTL_timeCount(unsigned int *tmTh);
extern void BTL_Updata(void);
extern void BTL_flashErase(unsigned char sector);
extern void BTL_memCpy(unsigned char *des, unsigned char const *src, unsigned int len);
extern void BTL_memClr(unsigned char *des,unsigned int len);
extern unsigned char BTL_IAPWrite(unsigned int addr,unsigned int line, unsigned char  xdata *date);
extern void BTL_flashInit(void);
extern void BTL_uartInit(void);
extern void BTL_timerInit(void);
extern void BTL_sendEcho(unsigned char content);
extern unsigned char BTL_setSysClock(unsigned char PLLClk);
#endif