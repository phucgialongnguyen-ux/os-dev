#include "sdst.h"
#define NULL ((void *)0)

typedef struct Header {
    size_tz size;
    int is_free;
    struct Header* next;
} Header;

extern Header* head; 

void thanks(void* ptr) {
    if (!ptr) return;
    Header* header = ((Header*)ptr) - 1;
    header->is_free = 1;
    Header* curr = head;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(Header) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}