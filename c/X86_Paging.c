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
                        | (AVL  << 8) 
                        | (PS   << 7) 
                        | (AVL  << 6) 
                        | (A    << 5)
                        | (PCD  << 4)
                        | (PWT  << 3)
                        | (UorS << 2)
                        | (RorW << 1)
                        | (P    << 0);
}
