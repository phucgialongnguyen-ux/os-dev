//source: https://wiki.osdev.org/Page_Frame_Allocation
//and
//source: https://wiki.osdev.org/Memory_Allocation#The_Big_Picture
#define NULL ((void *)0)
#include "sdst.h"
#include "size_t.h"

struct PMA{
    unsigned long long* memory_bit;
    unsigned long long* last_Allocated_Bit;
    unsigned int* buddy;
};
void Memory_Allocation(struct PMA* pma){
    unsigned long long index = 0;
    for(unsigned long long i = 0; i < 100; i++){
        if(((pma->memory_bit[index]) & (1ULL << i)) == 0){
            pma->memory_bit[index] |= (1ULL << i); 
            *(pma->last_Allocated_Bit) += 1;
            break;
        }
        else if(((pma->memory_bit[index]) & (1ULL << i)) != 0){
            unsigned long long* bitmap = pma->memory_bit;
            if(((bitmap[index]) & (1ULL << i)) == 0){
                unsigned long long* heap = (unsigned long long*)(4096);
                if(heap == NULL){
                    return;
                }
            }        
        }    
    }
}
