//#soucre https://wiki.osdev.org/X86_Paging#32-bit_Paging_(Protected_Mode)
//

#include "sdst.h"
#include "size_t.h"

_Alignas(4096) struct paging32bit{
    unsigned int page_directory_Entries[1024];
};

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
