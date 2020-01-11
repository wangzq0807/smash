#include "sys/types.h"
#include "utc.h"
#include "lib/log.h"
#include "asm.h"

#define CMOS_CMD_PORT   0x70
#define CMOS_DATA_PORT  0x71
#define READ_CMOS(reg)          \
({                              \
    outb(CMOS_CMD_PORT, 0x80|(reg)); \
    inb(CMOS_DATA_PORT);        \
})

#define BCD2BIN(val)    (((val)&0xF) + ((val)>>4)*10)

typedef struct _RTCTime    RTCTime;
struct _RTCTime {
    uint8_t     rt_seconds;
    uint8_t     rt_minutes;
    uint8_t     rt_hours;
    uint8_t     rt_day;
    uint8_t     rt_month;
    uint8_t     rt_year;
    uint8_t     rt_century;
};

void
setup_utc()
{
#ifdef KLOG_ENABLE
    RTCTime rtime;
    rtime.rt_seconds = READ_CMOS(0x0);
    rtime.rt_minutes = READ_CMOS(0x2);
    rtime.rt_hours = READ_CMOS(0x4);
    rtime.rt_day = READ_CMOS(0x7);
    rtime.rt_month = READ_CMOS(0x8);
    rtime.rt_year = READ_CMOS(0x9);
    rtime.rt_century = READ_CMOS(0x32);
    KLOG(DEBUG, "DATE: %d%d, %d, %d", BCD2BIN(rtime.rt_century),BCD2BIN(rtime.rt_year), BCD2BIN(rtime.rt_month), BCD2BIN(rtime.rt_day));
#endif // KLOG_ENABLE
}
