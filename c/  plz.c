#include "size_t.h"
#define NULL ((void *)0)
_Alignas(4096) static unsigned char heap[4096];
static size_tz heap_index = 0;
void* plz(size_tz size){
    size = (size + 7) & ~((size_tz)7);
    if(heap_index + size > 4096){
        return NULL;
    }
    void* ptr = &heap[heap_index];
    heap_index += size;
    return ptr;
}
