#include "bootloader.h"
extern void FV_UARTISR(void);
// 全局变量定义
//unsigned char xdata BTL_tempBuff[D_SECTORSIZE];
unsigned char xdata currentSector = 0;
//unsigned char xdata bootAddr[3];                                      // 用户更新用户程序时保存bootloader跳转指令及地址，防止bootloader入口被破坏
unsigned char xdata bootFlag[2] _at_ D_FLAGADDR;                        // 升级标志
// 用户代码主函数
#pragma ASM            
EXTRN CODE (?C_START)  
#pragma ENDASM  
/*************************************************************************
*函数功能：bootloader主函数入口
*参数：无
*返回值：无
*************************************************************************/
void BTL_main(void)
{
	D_DISINT();
	SPCFNC = 1;
	XBYTE[0x0402] = 0x86;
	SPCFNC = 0;
	D_CLRWDT();
	if (!BTL_setSysClock(4))                                            // 设置系统时钟13M
	{
		while (1);
	}                                                                   // 关闭全局中断
	D_CLRWDT();
	if (BTL_checkStatus())                                              // 判断是否处在升级状态
	{
		BTL_uartInit();                                                 // 初始化串口、定时器、flash。因为无法使用中断服务，所有的通讯、定时由查询完成
		BTL_timerInit();
		BTL_Updata();													// 进入升级
	}
	else
	{
		D_CLRWDT();
		#pragma ASM            
            LJMP    ?C_START       
        #pragma ENDASM
	}
}
/*************************************************************************
*函数功能：8#中断服务函数
*参数：无
*返回值：无
*************************************************************************/
void UARTCF1_ISR(void) interrupt 8                                      // UARTANDCF1INT_VECTOR
{
	if (EXIF & BIT4)
	{
		while ((ExInt2IFG & 0xFF) != 0)
		{
			if (ExInt2IFG & BIT6)                                       // FLASH擦写中断
			{
				ExInt2IFG &= (~BIT6);
			}
			else                                                        // 跳转到其他中断处理函数中
			{
				FV_UARTISR();
			}
		}
		EXIF &= (~BIT4);
	}
}
/*************************************************************************
*函数功能：内存拷贝函数
*参数：des，目的地址，src，源地址，len，数据长度
*返回值：viod
*************************************************************************/
void BTL_memCpy(unsigned char *des, unsigned char const * src, unsigned int len)
{
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		(*des) = (*src);
		des++;
		src++;
	}
}
/*************************************************************************
*函数功能：内存清零函数
*参数：des，目的地址，len，数据长度
*返回值：无
*************************************************************************/
void BTL_memClr(unsigned char *des, unsigned int len)
{
	unsigned int idata i;
	for (i = 0; i < len; i++)
	{
		(*des) = 0xFF;
		des++;
	}
}
/*************************************************************************
功能: 页擦除
参数: page,一页512字节
返回: void
***************************************************************************/
void BTL_pageerase0(unsigned int page)
{
	unsigned char xdata * p;
	SPC_FNC = 0x01;
	p = (unsigned char xdata*)0x0555;
	*p = 0xaa;
	p = (unsigned char xdata*)0x02aa;
	*p = 0x55;
	p = (unsigned char xdata*)0x0555;
	*p = 0x80;
	p = (unsigned char xdata*)0x0555;
	*p = 0xaa;
	p = (unsigned char xdata*)0x02aa;
	*p = 0x55;
	p = (unsigned char xdata *)page;
	*p = 0x30;
	SPC_FNC = 0x00;
	PCON = 0x01;
}
/*************************************************************************
功能: IAP写一字节
参数: addr写入地址 dat写入数据
返回: void
***************************************************************************/
void BTL_IAPWriteByte(unsigned int addr, unsigned char dat)
{
	unsigned char xdata * p;
	SPC_FNC = 0x01;
	p = (unsigned char xdata*)0x0555;
	*p = 0xaa;
	p = (unsigned char xdata*)0x02aa;
	*p = 0x55;
	p = (unsigned char xdata*)0x0555;
	*p = 0xa0;
	p = (unsigned char xdata *)addr;
	*p = dat;
	SPC_FNC = 0x00;
	PCON = 0x01;
}
/*************************************************************************
功能: IAP写入多字节
参数: addr 写入地址 date 数据指针  len 数据长度
返回: void
*备注：对flash进行写操作时，必须打开中断，否则会写入失败
***************************************************************************/
unsigned char BTL_IAPWrite(unsigned int addr, unsigned int line, unsigned char  xdata *date)
{
	unsigned int  i;
	unsigned char xdata buff[512];
	unsigned char code  *cp;
	unsigned char INTIEEAtmp;
	unsigned char EIEEAtmp;
	unsigned char IEEAtmp;
	bit EAtmp;
	unsigned char CBANKEAtmp;
	CBANKEAtmp = CBANK;

	EAtmp = EA;
	INTIEEAtmp = ExInt2IE;
	EIEEAtmp = EIE;
	IEEAtmp = IE;

	ExInt2IE = 0x40;
	IE = 0;
	EIE = 0x01;
	EA = 1;                                             // 任何其他中断发生都可能造成写入失败
	CBANK = 1;

	addr = addr << 9;

	for (i = 0; i < 0x200; i++)                         // 读取整页数据
	{
		cp = (unsigned char code *)(addr + i);
		buff[i] = (*cp);
	}

	BTL_pageerase0(addr);

	for (i = 0; i < D_UPDATALEN; i++)                   // 改写需要修改的数据
	{
		buff[line + i] = *(date + i);
	}

	for (i = 0; i < 0x200; i++)                          // 写入FLASH
	{
		BTL_IAPWriteByte(addr + i, buff[i]);
	}

	for (i = 0; i < 0x200; i++)                         // 如果写入的数据与收到的升级数据不同
	{
		cp = addr + i;
		if ((*cp) != buff[i])
		{
			ExInt2IE = INTIEEAtmp;
			EIE = EIEEAtmp;
			IE = IEEAtmp;
			EA = EAtmp;
			CBANK = CBANKEAtmp;
			return (D_FALSE);
		}
	}

	ExInt2IE = INTIEEAtmp;
	EIE = EIEEAtmp;
	IE = IEEAtmp;
	EA = EAtmp;
	CBANK = CBANKEAtmp;
	return (D_TRUE);
}
/*************************************************************************
*函数功能：升级状态检测
*参数：无
*返回值：无
*************************************************************************/
unsigned char BTL_checkStatus(void )
{

	if ((bootFlag[0] != 0x55) || (bootFlag[1] != 0x55))
	{
		return (0);
	}
	else
	{
		bootFlag[0] = 0;
		bootFlag[1] = 0;
		return (1);
	}

}
/*************************************************************************
*函数功能：升级进程
*参数：无
*返回值：无
*************************************************************************/
void BTL_Updata(void)
{
	unsigned char idata i = 0;                             
	unsigned char idata ptr = 0;                         
	unsigned char xdata uartBuff[D_FRAMELEN];
	unsigned int  time = 0;
	unsigned int  base = 0;
	unsigned char chk;
	unsigned char updateState = 0;
    unsigned char IAPRTL = 0;                           // IAP操作结果
//  不擦出第一页的情况下，就不要再保存
//  bootAddr[0] = *(unsigned char code *)(0x0000);      // should always equal 0x02
//  bootAddr[1] = *(unsigned char code *)(0x0001);
//  bootAddr[2] = *(unsigned char code *)(0x0002);      // 保存flash0x0000-0x0002三个字节的程序跳转信息

	BTL_sendEcho(D_BTL);
	ptr = 0;
	BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);                                  // 清空数据缓冲
	while (1)
	{
		D_CLRWDT();
		BTL_timeCount(&time);                                                           // 查询方式计时
		if (D_RISET())                                                                  // 查询标志位读取通信数据
		{
			D_RICLR();
			time = 0;                                                                   // 重置计时
			uartBuff[ptr] = D_SBUFF;
			ptr++;

			if (uartBuff[D_FRAMESTARTLC] != '#')
			{
				ptr = 0;
			}

			if (ptr >= D_FRAMELEN )
			{
				if (uartBuff[D_FRAMESTOPLC] == '#')                                     // 接收完整
				{
					chk = 0;
					for (i = D_SECTORLC; i < D_CHKLC; i++)                              // 计算校验码
					{
						chk += uartBuff[i];
					}
                    
					if (chk == uartBuff[D_CHKLC])                                        // hex校验正确
					{
						currentSector = uartBuff[D_SECTORLC];
						switch (uartBuff[D_FLAGLC])                                      // 处理数据帧
						{
                            case D_STARTUPDATE:
                            {
                                ptr = 0;
                                BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
                                BTL_sendEcho(D_START);                                      // 请求数据
                            }
                            break;
                            case D_UPDATE:                                                  // 接收升级数据
                            {
                                updateState = 1;                                            // 升级状态
                                base = D_UPDATALEN * uartBuff[D_LINELC];
                                IAPRTL = BTL_IAPWrite(currentSector, base, &uartBuff[D_DATESTARTLC]);
                                ptr = 0;
                                BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
                                if (IAPRTL)
                                {

                                    BTL_sendEcho(D_NEXTFRAME);
                                }
                                else
                                {
                                    BTL_sendEcho(D_REPEATFRAM);                              // 向上位机请求重发上一帧数据

                                }
                            }
                            break;

                            case D_ALLDATEDONE:                                               // 升级结束
                            {
                                BTL_sendEcho(D_UPDATADONE);
                                #pragma ASM            
                                    LJMP    ?C_START       
                                #pragma ENDASM
                            }
                            break;

                            case D_STOPUPDATE:                                                 // 强制结束升级,因为升级未完成，会每隔10秒请求一次升级
                            {
                                BTL_sendEcho(D_STOP);
                                updateState = 0;                                               // 清升级状态
                                ptr = 0;
                                BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);             // 清空数据缓冲
                                
                            }
                            break;

                            case D_EXITBTL:
                            {
                                BTL_sendEcho(D_EXIT);
                                #pragma ASM            
                                    LJMP    ?C_START       
                                #pragma ENDASM
                            }
                            break;

                            default:
                            {
                                ptr = 0;
                                BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
                                BTL_sendEcho(D_REPEATFRAM);
                            }
                            break;
						}
					}
					else
					{
						ptr = 0;
						BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
						BTL_sendEcho(D_REPEATFRAM);
					}
				}
				else
				{
					ptr = 0;
					BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
					BTL_sendEcho(D_REPEATFRAM);
				}
			}
		}
		else
		{
			if (time >= D_TIMETH)
			{
				time = 0;
				ptr = 0;
				BTL_memClr((unsigned char *)uartBuff, D_FRAMELEN);
				if (updateState == 0)                                                   // 不在升级状态
				{
					BTL_sendEcho(D_BTL);                                                // 向上位机请求数据，bootloader启动后，除非升级完成，否则不会退出
				}
				else
				{
					BTL_sendEcho(D_REPEATFRAM);                                        // 超时重发数据
				}
			}
		}
	}
}
/*************************************************************************
*函数功能：计时函数
*参数：tmth。计时变量指针
*返回值：无
*************************************************************************/
void BTL_timeCount(unsigned int *tmTh)
{
	if (D_TFSET())                                                      // 查询定时器
	{
		TH0 = 0x2A;
		TL0 = 0xAB;                                                     // 50ms定时
		D_TFCLR();
		(*tmTh)++;
	}
}
/*************************************************************************
*函数功能：返回帧
*参数：content,返回帧内容
*返回值：无
*************************************************************************/
void BTL_sendEcho(unsigned char content)
{
	unsigned char idata i;
	unsigned char idata echo[4];
	echo[0] = '#';
	echo[3] = '#';
	echo[1] = (unsigned char)(content / 10) + 0x30;
	echo[2] = (unsigned char)(content % 10) + 0x30;
	for (i = 0; i < 4; i++)
	{
		D_SBUFF = echo[i];
		while (!D_TISET());
		D_TICLR();

	}
}

/*************************************************************************
*函数功能：串口初始化
*参数：无
*返回值：无
*************************************************************************/
void BTL_uartInit(void )
{
	P1OE &= (~BIT4);
	P1IE &= (~BIT4);
	P1OE |= BIT3;
	P1IE |= BIT3;
	P13FS = 0x02;
	P14FS = 0x02;
	P1OD |= BIT4;

	TCON5 = 0xA2;                               // SMOD=0,UART波特率加倍，关闭波特率发生器
	SCON5 = 0x50;                               // 8位UART,UART4工作在模式1中,使能接收
	TMOD5 = 0x20;                               // 波特率发生器模式：自动重装初值8为定时器（兼容定时器1）
	TH51 = D_baud19200REG;
	ExInt3IE &= (~(BIT3 + BIT2));               // 禁止接收与发送中断
	EIE &= (~BIT1);                             // 禁止中断向量9
}
/*************************************************************************
*函数功能：定时器初始化
*参数：无
*返回值：无
*************************************************************************/
void BTL_timerInit(void )
{
	CKCON &= ~(BIT3);                         // 系统时钟作为时钟源，12分频
	TMOD  &= 0xF0;
	TMOD  |= 0x01;                            // 定时器0工作在方式1:16位向上计数器
	TH0 = 0x2A;
	TL0 = 0xAB;                               // 50ms定时

	TCON |= BIT4;                             // 开定时器0
	IE &= (~BIT1);                            // 允许中断
}
/*************************************************************************
*函数功能：系统时钟频率设置
*参数：无
*返回值：无
*************************************************************************/
unsigned char BTL_setSysClock(unsigned char PLLClk)
{
	unsigned char  i;
	i = 0;
	FSC = 0;
	FWC = 0;
	MCUFRQ = 0;                       //  设置时钟1的时钟源OSC
	while ((MCUFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);                // 在一定时间内没有锁定
		}
	}
	MEAFRQ = (0);                      // 设置时钟2的时钟源OSC
	while ((MEAFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);                // 在一定时间内没有锁定
		}
	}
	for (i = 0; i < 5; i++);
	CtrlCLK = 0xc0;                 // 开始PLL电路
	CtrlPLL = 0;                    // 800K pll 800k, DSP 800K,adc 200k,并切换到800kPLL
	while (!(PLLLCK & 0x01))         // 判断PLL时钟是否锁定
	{
		i++;
		if (i > 50)
		{
			return (0);               // 在一定时间内没有锁定
		}
	}

	MCUFRQ = 1;                    // 设置时钟1的时钟源PLL
	i = 0;
	while (!(MCUFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);             // 在一定时间内没有锁定
		}
	}
	MEAFRQ = 1;                   // 设置时钟2的时钟源PLL
	i = 0;
	while (!(MEAFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);            // 在一定时间内没有锁定
		}
	}
//  PMG=0;                         // 开启计量时钟
	switch (PLLClk)
	{
	case 0:                    // pll 800k, DSP 800K,adc 200k,并切换到800kPLL
	{
		CtrlCLK = 0xc0;
		CtrlPLL = 0x00;
	}
	break;

	case 1:                   // pll 1.6M, DSP 1.6M,adc 400k,并切换到1.6MPLL
	{
		CtrlCLK = 0xd5;
		CtrlPLL = 0x00;
	}
	break;

	case 2:                   // pll 3.2M, DSP 3.2M,adc 800k,并切换到3.2MPLL
	{
		CtrlCLK = 0xea;
		CtrlPLL = 0x00;
	}
	break;

	case 3:                  // pll 6.4M, DSP 3.2M,adc 800K,并切换到6.4MPLL
	{
		CtrlCLK = 0xeb;
		CtrlPLL = 0x00;
	}
	break;

	case 4:                  // pll 13M， DSP 3.2M,adc 800K,并切换到13MPLL
	{
		CtrlCLK = 0xeb;
		CtrlPLL = 0x40;
	}
	break;

	case 5:                  // pll 26M， DSP 3.2M,adc 800K,并切换到26MPLL
	{
		CtrlCLK = 0xeb;
		CtrlPLL = 0xc0;
	}
	break;

	default:                // pll 3.2M, DSP 3.2M,adc 800k,并切换到3.2MPLL
	{
		CtrlCLK = 0xea;
		CtrlPLL = 0x00;
	}
	break;
	}


	while (!(PLLLCK & 0x01))
	{
		i++;
		if (i > 50)
		{
			return (0);        // 在一定时间内没有锁定
		}
	}
	MCUFRQ = 1;
	i = 0;
	while (!(MCUFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);       // 在一定时间内没有锁定
		}
	}
	MEAFRQ = 1;
	i = 0;
	while (!(MEAFRQ))
	{
		i++;
		if (i > 20)
		{
			return (0);      // 在一定时间内没有锁定
		}
	}

//  PMG=0;                  // 开启计量时钟
	CtrlCry0 = 0;           // 调整启振波形
	CtrlCry1 = 3;
	CtrlCry2 |= 0x20;         // 使能起振电路停振复位


	CtrlBGP = 0x02;         // 改善高低温误差+10PPM
	CtrlM = 0x01;           // 禁止M通道去直
	if (CtrlCLK == 0xc0)       // 降频计量时 降低功耗
	{
		CtrlBGP |= 0xc0;    // 偏置电流减小66%
	}
	else
	{
		CtrlBGP &= (~(0xc0));
	}

	return (1);
}