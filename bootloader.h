#ifndef _BOOTLOADER_H
#define _BOOTLOAER_H
#include "inc\V9811.h"
#include <INTRINS.h>
/*�����ʼĴ�����ֵ*/
#define D_baud115200REG 0xF9
#define D_baud19200REG  0xD5
#define D_baud9600REG   0xAB
#define D_baud4800REG   0x55
#define D_baud2400REG   0xE4
#define D_baud1200REG   0xC7
#define D_baud600REG    0x8E
#define D_baud300REG    0x1C

// ��
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

// echo����
#define D_STOP         	0x00    // ��������
#define D_START         0x01    // ��ʼ��������ʱ��λ�����Է��͵�һ֡����

#define D_NEXTFRAME     0x02    // ������һ֡����
#define D_NEXTSECTOR    0x03    // ������һ��������

#define D_UPDATADONE    0x04    // �������
#define D_UPDATAFAIL    0x05    // ����ʧ��

#define D_REPEATFRAM    0x06    // �ظ�������һ֡����
#define D_REPEATSECTOR  0x07    // �ظ�������һ��������
#define D_BTL           0x08    // bootloader��־
#define D_EXIT          0x09    // �˳�bootloader

#define D_NOP()         _nop_()

#define D_V98XX         0
#define D_SH7012        1

// ȫ������
#define D_FLAGADDR      0x0FF0                          // xdata����������Ϊ������ȫ�ֱ�־
#define D_MCU           D_V98XX
#define D_UPDATALEN  	16							    // ÿ֡�������������ݳ���
#define D_FRAMELEN      (D_UPDATALEN+6)                 // ÿ֡ʵ�����ݳ���()
#define D_ADRESSMODE    D_ABSOLUTE					    // Ѱַģʽ
#define D_READCHECK		D_FALSE						    // �����ȶ�
#define D_TIMETH        (200)                            // *50ms

// �ؼ���ʶ�����±�
#define D_FRAMESTARTLC  0								// ֡ͷ
#define D_FRAMESTOPLC	(D_FRAMELEN-1)					// ֡β
#define D_SECTORLC		1								// ������
#define D_LINELC		2								// �к�
#define D_FLAGLC		3								// ��־λ(������������)
#define D_DATESTARTLC	4								// ���ݿ�ʼ
#define D_CHKLC			(D_FRAMELEN-2)					// У��λ

// �������Ʊ�ʶ
#define D_UPDATE 		0x00
#define D_SECTORDONE	0x01
#define D_ALLDATEDONE	0x02
#define D_STOPUPDATE	0x03
#define D_STARTUPDATE   0x04
#define D_EXITBTL       0x05
// note* bootloaderλ��flash�е����10k_(10��20ҳ)(0xD800-0xFFFF),����0xFE00ҳ���ڱ����û��������ڵ�ַ
#if (D_MCU == D_V98XX)
    #define D_SECTORSUM		0x6c						// 128-20ҳ
    #define D_SECTORSIZE	0x200 						// ÿҳ��С��V98xx��512���ֽ� ��ӱ7012��1024���ֽ�
#endif
#if (D_MCU == D_SH7012)
    #define D_SECTORSUM		0x36 						// 64-10ҳ
    #define D_SECTORSIZE/	0x400 						// ÿҳ��С��V98xx��512���ֽ� ��ӱ7012��1024���ֽ�
#endif

#define D_FRAMENUM		(D_SECTORSIZE/D_UPDATALEN)	    // ����һҳ������Ҫ����֡��

//// �û���������ڵ�ַ
//typedef void (code*USERAPP)(void);                      // ����ָ��
//#define D_MAINBASEADDR     (0x0600)

// ��������


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