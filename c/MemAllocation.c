//source: https://wiki.osdev.org/Page_Frame_Allocation
//and
//source: https://wiki.osdev.org/Memory_Allocation#The_Big_Picture
#define NULL ((void *)0)
#include "sdst.h"
#include "size_t.h"

struct PMA{
    unsigned long long* memory_bit;
    unsigned long long last_Allocated_Bit;
    unsigned long long* buddy;
    unsigned long long last_Allocated_Buddy_Bit;
};

unsigned long long Memory_Allocation(struct PMA* pma){
    if(pma == NULL || pma->memory_bit == NULL || pma->buddy == NULL){
        return 0;
    }

    unsigned long long index = 0;
    unsigned long long buddy_index = 0;

    for(size_tz i = 0; i < 64; i++){
        if(((pma->memory_bit[index]) & (1ULL << i)) == 0 && ((pma->buddy[buddy_index]) & (1ULL << i)) == 0){
            pma->memory_bit[index] |= (1ULL << i); 
            pma->buddy[buddy_index] |= (1ULL << i);

            pma->last_Allocated_Bit += 1;
            pma->last_Allocated_Buddy_Bit += 1;

            unsigned long long physc_addr = (buddy_index * 64 + i) * 4096;
            return physc_addr;
        }
    }

    return 0;
}