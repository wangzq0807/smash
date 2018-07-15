#ifndef __PARTION__
#define __PARTION__
#include "sys/types.h"

typedef struct _PartionEntity PartionEntity;
struct _PartionEntity {
    uint8_t     pe_indicator;
    uint8_t     pe_start_header;
    uint8_t     pe_start_sector;
    uint8_t     pe_start_cylinder;
    uint8_t     pe_system_id;
    uint8_t     pe_end_header;
    uint8_t     pe_end_sector;
    uint8_t     pe_end_cylinder;
    uint32_t    pe_lba_start;
    uint32_t    pe_total_sector;
};

error_t init_partion(dev_t dev);

PartionEntity * get_partion_entity(dev_t i);


#endif // __PARTION__