//#soucre https://wiki.osdev.org/X86_Paging#32-bit_Paging_(Protected_Mode)

#include "sdst.h"
#include "size_t.h"

_Alignas(4096) struct Paging64Bit4KB {
    unsigned long long entries[512];
};

void get_paging_index(unsigned long long virtual_addr){
    unsigned long long PML4 = virtual_addr;
    PML4 = (PML4 >> 39) & 0x1FF;
    unsigned long long PDPT = virtual_addr;
    PDPT = (PDPT >> 30) & 0x1FF;
    unsigned long long PD = virtual_addr;
    PD = (PD >> 21) & 0x1FF;
    unsigned long long PT = virtual_addr;
    PT = (PT >> 12) & 0x1FF;
    unsigned long long Page_offset = virtual_addr;
    Page_offset = Page_offset & 0xFFF;                                                                      
}                                                                               

void map_page(struct Paging64Bit4KB *pml4,unsigned long long virtual_addr,unsigned long long phys_addr, unsigned int flags){   
    unsigned long long PML4_idx = virtual_addr;
    PML4_idx = (PML4_idx >> 39) & 0x1FF;
    unsigned long long PDPT_idx = virtual_addr;
    PDPT_idx = (PDPT_idx >> 30) & 0x1FF;
    unsigned long long PD_idx = virtual_addr;
    PD_idx = (PD_idx >> 21) & 0x1FF;
    unsigned long long PT_idx = virtual_addr;
    PT_idx = (PT_idx >> 12) & 0x1FF;
    unsigned long long Page_offset_idx = virtual_addr;
    Page_offset_idx = Page_offset_idx & 0xFFF; 

    struct Paging64Bit4KB *pdpt;
    struct Paging64Bit4KB *pd;
    struct Paging64Bit4KB *pt;
    
    unsigned long long entry_pml4 = pml4->entries[PML4_idx];
        if(!(entry_pml4 & 1)){
            struct Paging64Bit4KB *new_table = (struct Paging64Bit4KB *)plz(4096);
            for(int i = 0; i < 512; i++){
                new_table->entries[i] = 0;
            }
            pml4->entries[PML4_idx] = (unsigned long long)new_table | flags | 1;
        }
    
    pdpt = (struct Paging64Bit4KB *)(pml4->entries[PML4_idx] & 0x000FFFFFFFFFF000ULL);
    unsigned long long entry_pdpt = pdpt->entries[PDPT_idx];
        if(!(entry_pdpt & 1)){
            struct Paging64Bit4KB *new_table = (struct Paging64Bit4KB *)plz(4096);
            for(int i = 0; i < 512; i++){
                new_table->entries[i] = 0;
            }
            pdpt->entries[PDPT_idx] = (unsigned long long)new_table | flags | 1;
        }
    pd = (struct Paging64Bit4KB*)(pdpt->entries[PDPT_idx] & 0x000FFFFFFFFFF000ULL);
    unsigned long long entry_pd = pd->entries[PD_idx];
        if(!(entry_pd & 1)){
            struct Paging64Bit4KB *new_table = (struct Paging64Bit4KB*)plz(4096);
              for(int i = 0; i < 512; i++){
                new_table->entries[i] = 0;
            }
            pd->entries[PD_idx] = (unsigned long long)new_table | flags | 1;
        }
    pt = (struct Paging64Bit4KB*)(pd->entries[PD_idx] & 0x000FFFFFFFFFF000ULL);
    pt->entries[PT_idx] = (phys_addr & 0x000FFFFFFFFFF000ULL) | flags | 1;
}

//maybe i'll finish this one tomorrow, or longer(next week)
//i hate school so much!