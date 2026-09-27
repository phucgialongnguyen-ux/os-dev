#include "sdst.h"
#include "size_t.h"

_Alignas(4096) struct Paging32Bit4MB {
    unsigned int entries[1024];
};

// Hàm tạo PDE 4MB (thay cho Page_Directory cũ)
unsigned int make_pde_4mb(unsigned int phys_addr, unsigned int flags) {
    unsigned int device = phys_addr & 0xFFC00000; 
    unsigned int PS = 1;                          
    unsigned int P = 1;                           

    return device | (PS << 7) | (flags & 0xFFF) | P;
}

static inline void enable_pse(void) {
    unsigned int cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 4); 
    asm volatile("mov %0, %%cr4" : : "r"(cr4));
}

static inline void invlpg(unsigned long addr) {
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static inline void load_page_directory(unsigned int *pd_phys_addr) {
    asm volatile("mov %0, %%cr3" : : "r"(pd_phys_addr));
}

static inline void enable_paging(void) {
    unsigned int cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}