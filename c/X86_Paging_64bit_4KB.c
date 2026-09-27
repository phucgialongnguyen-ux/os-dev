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
    if((pml4->entries[PML4_idx] & 1) == 0){
        pml4->entries[PML4_idx] = pml4->entries[PML4_idx] & 0x000FFFFFFFFFF000ULL;
        //Comming soon, i'm busy as fu#k
    }                                                                                                                                                                                                                                                                                          
}                                                                                                                                                                                                                                                                                                                    

