#include "sdst.h" 
#define NULL ((void *)0)
typedef struct Header {
    size_tz size;
    int is_free;
    struct Header* next;
} Header;
_Alignas(4096) static unsigned char heap[524288];
static Header* head = NULL;
void init_heap(void) {
    head = (Header*)heap;
    head->size = sizeof(heap) - sizeof(Header);
    head->is_free = 1;
    head->next = NULL;
}
void* plz(size_tz size) {
    if (!head) init_heap();
    if (size == 0) return NULL;
    size = (size + 7) & ~((size_tz)7);
    Header* curr = head;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size >= size + sizeof(Header) + 16) {
                Header* next_block = (Header*)((unsigned char*)curr + sizeof(Header) + size);
                next_block->size = curr->size - size - sizeof(Header);
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = size;
                curr->next = next_block;
            }
            curr->is_free = 0;
            return (void*)(curr + 1); 
        }
        curr = curr->next;
    }
    return NULL; 
}
