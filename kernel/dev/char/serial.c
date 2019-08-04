#include "serial.h"
#include "asm.h"

/****************
 * 中断寄存器 PORT+1
 ****************/
#define IF_DISABLE      0   // 禁止中断
#define IF_ENABLE       1   // 允许中断
#define IF_EMPTY        2   // 空
#define IF_BREAK        4   // 中断/错误
#define IF_STATUS       8   // 状态变更

/*****************
 * 控制寄存器 PORT+3
 *****************/
/*------------------------------------------------------------------
Bit7	
    | 1	| Divisor Latch Access Bit
    | 0	| Access to Receiver buffer, Transmitter buffer & Interrupt Enable Register
--------------------------------------------------------------------
Bit6 forces TxD +12V (break)
--------------------------------------------------------------------
Bits 3, 4 And 5	| Parity Select
--------------------------------------------------------------------
    | X	| X	| 0	| No Parity
    | 0	| 0	| 1	| Odd Parity
    | 0	| 1	| 1	| Even Parity
    | 1	| 0	| 1	| High Parity (Sticky)
    | 1	| 1	| 1	| Low Parity (Sticky)
--------------------------------------------------------------------
Bit 2 Length of Stop Bit
--------------------------------------------------------------------
    | 0	| One Stop Bit
    | 1	| 2 Stop bits for words of length 6,7 or 8 bits or 1.5 Stop Bits for Word lengths of 5 bits.
--------------------------------------------------------------------
Bits 0 And 1	Bit 1	Bit 0	Word Length
--------------------------------------------------------------------
    | 0	| 0	| 5 | Bits
    | 0	| 1	| 6 | Bits
    | 1	| 0	| 7 | Bits
    | 1	| 1	| 8 | Bits
Line Control Register
*-----------------------------------------------------------------*/

#define DLAP_BIT     0x80

/****************
 * 状态寄存器 PORT+5
 ****************/
enum ComStatus {
    Data_Ready,
    Over_Run,
    Parity_Error
};

int nComPort = COM_PORT1;

static inline void _set_rate(ComRate rate)
{
    outb(rate & 0xff, nComPort + 0);
    outb(rate >> 8, nComPort + 1);
}

static inline void _set_proto(ComDataBits dbits, ComStopBits sbits )
{
    outb(dbits|sbits, nComPort + 3);
}

void init_serial(int port)
{
    nComPort = port;
    outb(IF_DISABLE, nComPort+1);
    {
        outb(DLAP_BIT, nComPort + 3);           // 设置DLAP
        _set_rate(ComR57600);                   // 设置波特率
        outb(0, nComPort + 3);                  // 清除DLAP
        _set_proto(ComDataBits8, ComStopBits1); // 8bit数据, 1bit停止
    }
    outb(IF_ENABLE, nComPort+1);
}

void set_rate(ComRate rate)
{
    outb(DLAP_BIT, nComPort + 3);    // 设置DLAP
    _set_rate(rate);
    outb(0, nComPort + 3);           // 清除DLAP
}

void write_serial(char a)
{
    outb(a, nComPort);
}