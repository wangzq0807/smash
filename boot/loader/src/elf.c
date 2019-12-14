
#include "elf.h"
#include "string.h"
#include "fs/file.h"
#include "fs/nodes.h"
#include "log.h"
#include "memory.h"

#define PT_NULL     0       /* Program header table entry unused */                                                                                                                           
#define PT_LOAD     1       /* Loadable program segment */

// TODO: 暂定16个Program header
#define PGH_NUM 16
Elf32_Ehdr  elfHeader;
Elf32_Phdr  progHeader[PGH_NUM]; 

int
LoadElfProg(const IndexNode *inode, const uint32_t offset, const uint32_t phnum)
{
    KLOG(DEBUG, "LoadElfProg start");
    if (phnum >= PGH_NUM)
        KLOG(ERROR, "Program header num is more than %d", PGH_NUM);
    Elf32_Phdr* phdr = (Elf32_Phdr*)&progHeader[0];
    file_read(inode, offset, phdr, phnum*sizeof(Elf32_Phdr));
    for (int phi = 0; phi < phnum; ++phi)
    {
        Elf32_Phdr* cur_phdr = phdr + phi;
        if (cur_phdr->p_type != PT_LOAD)
            continue; // 只读取可加载的段

        if (cur_phdr->p_paddr == 0)
            continue;
        Elf32_Addr paddr = cur_phdr->p_paddr;   // 物理地址
        Elf32_Word memsz = cur_phdr->p_memsz;   // 占内存大小
        Elf32_Off fileOff = cur_phdr->p_offset; // 在文件中的偏移
        KLOG(DEBUG, "phinfo %X %X %X", fileOff, paddr, memsz);
        Elf32_Word adjustsz = 0;
        if (cur_phdr->p_align > 1)
        {
            paddr = paddr / cur_phdr->p_align * cur_phdr->p_align;
            fileOff = fileOff / cur_phdr->p_align * cur_phdr->p_align;
            adjustsz = cur_phdr->p_paddr - paddr;
            memsz += adjustsz;
        }
        const int readsz = cur_phdr->p_filesz + adjustsz;
        file_read(inode, fileOff, (void *)paddr, readsz);

        void *bssAddr = ((void *)paddr) + readsz;
        const int bssSz = memsz - readsz;
        if (bssSz > 0)
            memset(bssAddr, 0, bssSz);
    }
    KLOG(DEBUG, "LoadElfProg end");
    return 0;
}

vm_t
LoadElf(const IndexNode *inode)
{
    Elf32_Ehdr* ehdr = &elfHeader;
    int rdsize = file_read(inode, 0, ehdr, sizeof(Elf32_Ehdr));
    if (rdsize == 0)
        return -1;
    const Elf32_Addr retAddr = ehdr->e_entry;
    KLOG(DEBUG, "LoadElf %X  phnum: %d", retAddr, ehdr->e_phnum);

    // 加载program
    if (ehdr->e_phoff != 0)
    {
        LoadElfProg(inode, ehdr->e_phoff, ehdr->e_phnum);
    }
    
    return (vm_t)retAddr;
}

