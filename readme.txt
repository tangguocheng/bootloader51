# ��ַ����
-0x0000-0x01FF: �ж�������
-0x0200-0x03FF: ����
-0x0400-0x05FF: 512�ֽڵĳ�����������Ƭ����������������Ĳ������������û����롣
-0x0600-0xD5FF: �û�����
-0xD600-0xD7FF: �ж�ӳ�����
-0xD800-0xD8FF: ��������
-0xD900-0xFFFF: bootloader����

��ΪIAP������Ҫʹ���жϣ�����ᵼ��MCU���ޱ��������Ҫ���һҳ����������ݲ��ܱ�������
��ˣ���Ҫ���жϷ����������ȫ�����þ��Զ�λ
�ڴ��������:(�ɸ���ʵ����������޸�)
     |----SR---|---�ڴ����--|
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
	 
���У���UART1SEG��UARTCFSEG����512���ֽڣ���������256�ֽڡ��������#8���жϣ���Ϊ��IAP�жϹ��ã���IAP�жϴ���
���ܲ�������˶���UARTCFSEGʵ���ϲ������жϷ������������ɰ˺��жϷ��������õ��Ӻ�����ʵ��λ�á�

����֡��ʽ

|----֡ͷ----|----������----|----��������----|----У���----|----֡β----|
|----  # ----|----Sector----|----  date  ----|----  Chk ----|---- #  ----|

����У��͵ļ��㷽����
     Chk = 0x100-(sector+date)(�޽�λ)

����֡��ʽ

|----֡ͷ----|----���ش���----|----֡β----|
|----  # ----|----���ش���----|---- #  ----|

```
// echo����
#define D_START         0x01    // ��ʼ��������ʱ��λ�����Է��͵�һ֡����

#define D_NEXTFRAME     0x02    // ������һ֡����
#define D_NEXTSECTOR    0x03    // ������һ��������

#define D_UPDATADONE    0x04    // �������
#define D_UPDATAFAIL    0x05    // ����ʧ��

#define D_REPEATFRAM    0x06    // �ظ�������һ֡����
#define D_REPEATSECTOR  0x07    // �ظ�������һ��������
#define D_BTL           0x08    // bootloader��־
```