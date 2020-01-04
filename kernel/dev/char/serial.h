#pragma once

// COM 端口号
#define COM_PORT1   0x3F8
#define COM_PORT2   0x2F8
#define COM_PORT3   0x3E8
#define COM_PORT4   0x2E8

typedef enum _ComRate
{
    ComR115200 = 1,
    ComR57600 = 2,
    ComR38400 = 3,
    ComR28800 = 4
} ComRate;

typedef enum _ComDataBits
{
    ComDataBits5 = 0,
    ComDataBits6 = 1,
    ComDataBits7 = 2,
    ComDataBits8 = 3
} ComDataBits;

typedef enum _ComStopBits
{
    ComStopBits1 = 0,
    ComStopBits2 = 4
} ComStopBits;

void setup_serial(int port);

void serial_write(int port, char a);
