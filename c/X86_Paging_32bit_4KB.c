//#soucre https://wiki.osdev.org/X86_Paging#32-bit_Paging_(Protected_Mode)
//

#include "sdst.h"
#include "size_t.h"

_Alignas(4096) struct paging32bit{
    unsigned int page_directory_Entries[1024];
};

//Hàm tạo PDE 4KB
void Page_Directory(unsigned int phys_addr){
    unsigned int device = phys_addr & 0xFFFFF000;
    unsigned int AVL;
    unsigned int PS;
    unsigned int A;
    unsigned int PCD;
    unsigned int PWT;
    unsigned int UorS;
    unsigned int RorW;
    unsigned int P = 0;
    unsigned int entry = device 
                        | (AVL  << 9) 
                        | (PS   << 7) 
                        | (A    << 5)
                        | (PCD  << 4)
                        | (PWT  << 3)
                        | (UorS << 2)
                        | (RorW << 1)
                        | (P    << 0);
    
    
}

_Alignas(4096) struct Page_Table{
    unsigned int page_table_Entries[1024];
};

void PTE(unsigned int phys_addr){
    unsigned int device = phys_addr & 0xFFFFF000;
    unsigned int AVL;
    unsigned int G;
    unsigned int PAT;
    unsigned int D;
    unsigned int A;
    unsigned int PCD;
    unsigned int PWT;
    unsigned int UorS;
    unsigned int RorW;
    unsigned int P;
    unsigned int entry = device
                        | (AVL << 9)
                        | (G << 8)
                        | (PAT << 7)
                        | (D << 6)
                        | (A << 5)
                        | (PCD << 4)
                        | (PWT << 3)
                        | (UorS << 2)
                        | (RorW << 1)
                        | (P << 0);
}

void *get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long *pd = (unsigned long *)0xFFFFF000;

    unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags) {

    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long *pd = (unsigned long *)0xFFFFF000;

    unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    
    pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01;

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