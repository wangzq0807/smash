
#include "elf.h"
#include "sys/mman.h"
#include "string.h"
#include "memory.h"
#include "fs/file.h"

#define PT_NULL     0       /* Program header table entry unused */                                                                                                                           
#define PT_LOAD     1       /* Loadable program segment */
#define PT_DYNAMIC  2       /* Dynamic linking information */
#define PT_INTERP   3       /* Program interpreter */
#define PT_NOTE     4       /* Auxiliary information */
#define PT_SHLIB    5       /* Reserved */
#define PT_PHDR     6       /* Entry for header table itself */
#define PT_LOPROC   0x70000000  /* Start of processor-specific */
#define PT_HIPROC   0x7fffffff  /* End of processor-specific */

#define PF_X        (1 << 0)    /* Segment is executable */
#define PF_W        (1 << 1)    /* Segment is writable */
#define PF_R        (1 << 2)    /* Segment is readable */

int
ToMMapProt(Elf32_Word pflags)
{
    int nRet = 0;
    if (pflags & PF_R)
        nRet |= PROT_READ;
    if (pflags & PF_W)
        nRet |= PROT_WRITE;
    if (pflags & PF_X)
        nRet |= PROT_EXEC;
    return nRet;
}

int
LoadElfProg(int fd, const IndexNode *inode, uint32_t offset, uint32_t num)
{
    Elf32_Phdr* phdr = (Elf32_Phdr*)vm_alloc();
    file_read(inode, offset, phdr, num*sizeof(Elf32_Phdr));
    for (int i = 0; i < num; ++i)
    {
        Elf32_Phdr* cur_phdr = phdr + i;
        switch (cur_phdr->p_type)
        {
        case PT_LOAD:
        {
            if (cur_phdr->p_vaddr == 0)
                break;
            //int prot = ToMMapProt(cur_phdr->p_flags);
            Elf32_Addr vaddr = cur_phdr->p_vaddr;
            Elf32_Word memsz = cur_phdr->p_memsz;
            Elf32_Off fileOff = cur_phdr->p_offset;
            Elf32_Word alignsz = 0;
            if (cur_phdr->p_align > 1)
            {
                vaddr = vaddr / cur_phdr->p_align * cur_phdr->p_align;
                fileOff = fileOff / cur_phdr->p_align * cur_phdr->p_align;
                alignsz = cur_phdr->p_vaddr - vaddr;
                memsz += alignsz;
            }
            void* pmap = (void*)vm_map_file((vm_t)vaddr, memsz, fd, fileOff);
            if (pmap == MAP_FAILED) {
                // printf("Error: %s\n", strerror(errno));
            }
            //int bsssize = cur_phdr->p_memsz - cur_phdr->p_filesz;
            //memset((void *)pmap + alignsz + cur_phdr->p_filesz, 0, bsssize);
        }
            break;
        case PT_INTERP:
        case PT_DYNAMIC:
        case PT_PHDR:
        {

        }
        
        default:
            break;
        }
    }
    vm_free(phdr);
    return 0;
}


typedef enum _ElfType
{
    ET_NONE,    // No file type
    ET_REL,     // Relocatable file
    ET_EXEC,    // Executable file
    ET_DYN,     // Shared object file
    ET_CORE,    // Core file
    ET_LOPROC = 0xff00, // Processor-specific
    ET_HIPROC = 0xffff // Processor-specific
} ElfType;

#define ELF_MAGIC   0x464C457F
#define ISA_x86     3
#define ISA_x86_64  0x3E

void ElfLog(int level, const char* str)
{
    //printf("%s\n", str);
};

typedef void (*startFunc)();

uint32_t
LoadElf(int fd, const IndexNode *inode)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)vm_alloc();
    file_read(inode, 0, ehdr, sizeof(Elf32_Ehdr));
    /* 检查elf iednt
    if (ph->eh_magic != ELF_MAGIC) {
        ElfLog(1, "elf magic error.");
    }

    if (ph->e_type != ET_EXEC) {
        ElfLog(1, "elf not exe");
    }*/

    // 加载program
    if (ehdr->e_phoff != 0)
    {
        LoadElfProg(fd, inode, ehdr->e_phoff, ehdr->e_phnum);
    }
    // 加载section
    if (ehdr->e_shoff != 0)
    {
        // LoadElfSect(fd, ehdr->e_shoff, ehdr->e_shnum);
    }
    uint32_t retAddr = ehdr->e_entry;
    vm_free(ehdr);
    
    return retAddr;
}

