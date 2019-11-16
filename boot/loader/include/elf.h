#ifndef __ELF_H__
#define __ELF_H__
#include "types.h"
#include "fs/file.h"

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;


#define EI_NIDENT (16)

typedef struct {
    unsigned char e_ident[EI_NIDENT];

    Elf32_Half      e_type;         // 1:可重定位 2:可执行 3:共享 4:核心
    Elf32_Half      e_machine;      // 指令集。3:Intel 386
    Elf32_Word      e_version;      // elf版本
    Elf32_Addr      e_entry;        // 入口点
    Elf32_Off       e_phoff;        // 应用程序头偏移

    Elf32_Off       e_shoff;        // sect偏移
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;       // elf头大小
    Elf32_Half      e_phentsize;    // prog头大小
    Elf32_Half      e_phnum;        // prog数量
    Elf32_Half      e_shentsize;    // sect大小

    Elf32_Half      e_shnum;        // sect数量
    Elf32_Half      e_shstrndx;     // string table 索引
} Elf32_Ehdr;

// Program Header
typedef struct
{
    Elf32_Word    p_type;         /* Segment type */
    Elf32_Off     p_offset;       /* Segment file offset */
    Elf32_Addr    p_vaddr;        /* Segment virtual address */
    Elf32_Addr    p_paddr;        /* Segment physical address */
    Elf32_Word    p_filesz;       /* Segment size in file */
    Elf32_Word    p_memsz;        /* Segment size in memory */
    Elf32_Word    p_flags;        /* Segment flags */
    Elf32_Word    p_align;        /* Segment alignment */
} Elf32_Phdr;

// Section Header
typedef struct
{
    Elf32_Word    sh_name;        /* Section name (string tbl index) */
    Elf32_Word    sh_type;        /* Section type */
    Elf32_Word    sh_flags;       /* Section flags */
    Elf32_Addr    sh_addr;        /* Section virtual addr at execution */
    Elf32_Off     sh_offset;      /* Section file offset */
    Elf32_Word    sh_size;        /* Section size in bytes */
    Elf32_Word    sh_link;        /* Link to another section */
    Elf32_Word    sh_info;        /* Additional section information */
    Elf32_Word    sh_addralign;       /* Section alignment */
    Elf32_Word    sh_entsize;     /* Entry size if section holds table */
} Elf32_Shdr;


vm_t
LoadElf(const IndexNode *inode);

#endif
