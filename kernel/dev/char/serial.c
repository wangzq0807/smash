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
Bit 0 And Bit 1	 Word Length
--------------------------------------------------------------------
    | 0	| 0	| 5 Bits
    | 0	| 1	| 6 Bits
    | 1	| 0	| 7 Bits
    | 1	| 1	| 8 Bits
Line Control Register
*-----------------------------------------------------------------*/

#define DLAP_BIT     0x80

/****************
 * 状态寄存器 PORT+5
 ****************/
/* Line status register
The line status register is useful to check for errors and enable polling.

Bit	Name	            Meaning
0	Data ready (DR)	    Set if there is data that can be read
1	Overrun error (OE)	Set if there has been data lost
2	Parity error (PE)	Set if there was an error in the transmission as detected by parity
3	Framing error (FE)	Set if a stop bit was missing
4	Break indicator (BI)	Set if there is a break in data input
5	Transmitter holding register empty (THRE)	Set if the transmission buffer is empty (i.e. data can be sent)
6	Transmitter empty (TEMT)	Set if the transmitter is not doing anything
7	Impending Error	    Set if there is an error with a word in the input buffer
*/

int nComPort = COM_PORT1;

void setup_serial(int port)
{
    nComPort = port;
    outb(nComPort+1, IF_DISABLE);
    outb(nComPort+2, 0);            // 禁止FIFO
    outb(nComPort+3, 3);            // 8bit数据, 无校验, 1bit停止
    {
        // 设置波特率
        outb(nComPort + 3, DLAP_BIT);   // 设置DLAP
        //_set_rate(ComR57600);
        outb(nComPort + 0, 1);
        outb(nComPort + 1, 0);
        outb(nComPort + 3, 0);          // 清除DLAP
    }
    outb(nComPort+1, IF_ENABLE);
}

void set_rate(ComRate rate)
{
    outb(nComPort + 3, DLAP_BIT);    // 设置DLAP
    //_set_rate(rate);
    outb(nComPort + 3, 0);           // 清除DLAP
}

int is_transmit_empty()
{
    return inb(nComPort+5) & 0x20;
}

void write_serial(char c)
{
    while (is_transmit_empty() == 0);
    
    outb(nComPort, c);
}