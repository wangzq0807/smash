#ifndef __ELF_H__
#define __ELF_H__
#include "sys/types.h"

#define ELF_MAGIC   0x464C457F
#define ISA_x86     3
#define ISA_x86_64  0x3E

typedef struct _ElfHeader ElfHeader;
struct _ElfHeader {
    uint32_t    eh_magic;           // 0x7f,"ELF"
    uint8_t     eh_bits;            // 1: 32; 2:64
    uint8_t     eh_endian;          // 1: little; 2: big
    uint8_t     eh_version;         // elf版本
    uint8_t     eh_os;              // 0: system v
    uint8_t     eh_padding[8];      // 未使用

    uint16_t    eh_type;            // 1:可重定位 2:可执行 3:共享 4:核心
    uint16_t    eh_isa;             // 指令集。
    uint32_t    eh_version_ex;      // elf版本
    uint32_t    eh_entry;           // 入口点
    uint32_t    eh_prog_header;     // 应用程序头

    uint32_t    eh_sect_header;     // 节
    uint32_t    eh_flags;
    uint16_t    eh_header_size;     // 本头大小
    uint16_t    eh_prog_hsize;      // 
    uint16_t    eh_prog_hnum;
    uint16_t    eh_sect_hsize;      //

    uint16_t    eh_sect_hnum;
    uint16_t    eh_index;
};

typedef struct _ProgHeader ProgHeader;
struct _ProgHeader {
    uint32_t    ph_type;
    uint32_t    ph_offset;
    uint32_t    ph_vaddr;
    uint32_t    ph_pyaddr;
    uint32_t    ph_filesize;
    uint32_t    ph_memsize;
    uint32_t    ph_flags;
    uint32_t    ph_aligh;
};

typedef struct _SectHeader SectHeader;
struct _SectHeader {
    uint32_t    sh_name;
    uint32_t    sh_type;
    uint32_t    sh_flags;
    uint32_t    sh_addr;
    uint32_t    sh_offset;
    uint32_t    sh_size;
    uint32_t    sh_link;
    uint32_t    sh_info;
    uint32_t    sh_aligh;
    uint32_t    sh_entsize;
};

#endif
