# 地址分配
-0x0000-0x01FF: 中断向量表
-0x0200-0x03FF: 保留
-0x0400-0x05FF: 512字节的出厂参数，单片机正常工作所必须的参数，不用于用户代码。
-0x0600-0xD5FF: 用户代码
-0xD600-0xD7FF: 中断映射代码
-0xD800-0xD8FF: 启动代码
-0xD900-0xFFFF: bootloader代码

因为IAP擦除需要使用中断，否则会导致MCU无限被挂起，这就要求第一页里得所有内容不能被擦出。
因此，需要将中断服务函数的入口全部采用绝对定位
内存分配如下:(可根据实际情况再做修改)
     |----SR---|---内存分配--|
     EXINT0SEG    =0xCA00-0xCAFF
     TIMER0SEG    =0xCB00-0xCBFF
     EXINT1SEG    =0xCC00-0xCCFF
     TIMER1SEG    =0xCD00-0xCDFF
     TIMER2SEG    =0xCE00-0xCEFF
     UART1SEG     =0xCF00-0xD1FF
    *UARTCFSEG    =0xD200-0xD3FF
     UARTRTCSEG   =0xD400-0xD4FF
     PLLEXINT3SEG =0xD500-0xD5FF
     TIMERASEG    =0xD600-0xD6FF
     POWERSEG     =0xD700-0xD7FF
	 
其中，除UART1SEG、UARTCFSEG分配512个字节，其他分配256字节。另外对于#8号中断，因为与IAP中断共用，而IAP中断代码
不能擦除，因此对于UARTCFSEG实际上并不是中断服务函数，而是由八号中断服务函数调用的子函数的实际位置。

升级帧格式

|----帧头----|----扇区号----|----升级数据----|----校验和----|----帧尾----|
|----  # ----|----Sector----|----  date  ----|----  Chk ----|---- #  ----|

其中校验和的计算方法：
     Chk = 0x100-(sector+date)(无进位)

返回帧格式

|----帧头----|----返回代码----|----帧尾----|
|----  # ----|----返回代码----|---- #  ----|

```
// echo代码
#define D_START         0x01    // 开始升级，此时上位机可以发送第一帧数据

#define D_NEXTFRAME     0x02    // 请求下一帧数据
#define D_NEXTSECTOR    0x03    // 请求下一扇区数据

#define D_UPDATADONE    0x04    // 升级完成
#define D_UPDATAFAIL    0x05    // 升级失败

#define D_REPEATFRAM    0x06    // 重复请求上一帧数据
#define D_REPEATSECTOR  0x07    // 重复请求上一扇区数据
#define D_BTL           0x08    // bootloader标志
```
