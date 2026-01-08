#include "memory_dump.h"
#include "../allocator/allocator.h"
#include "../allocator/buddy.h"
#include <stdio.h>
#include <stdint.h>

/* Buddy block header */
typedef struct {
    uint32_t id;              /* 0 if free */
    uint32_t order;           /* block size = 2^order */
    uint32_t requested_size;
} buddy_hdr_t;

void memory_dump(void)
{
    printf("\n========== MEMORY DUMP ==========\n");

    if (get_allocator_algo() == ALGO_BUDDY) {

        uint8_t *heap_start = buddy_get_base();
        size_t   heap_size  = buddy_get_size();

        size_t total_used = 0;
        size_t total_free = 0;

        uintptr_t ptr = (uintptr_t)heap_start;
        uintptr_t end = ptr + heap_size;

        while (ptr < end) {

            buddy_hdr_t *hdr = (buddy_hdr_t *)ptr;
            size_t size = (1UL << hdr->order);

            uintptr_t start = ptr;
            uintptr_t stop  = ptr + size - 1;

            if (hdr->id == 0) {
                printf("[0x%016lx - 0x%016lx] FREE (%zu bytes)\n",
                       (unsigned long)start,
                       (unsigned long)stop,
                       size);
                total_free += size;
            } else {
                printf("[0x%016lx - 0x%016lx] USED (%zu bytes)\n",
                       (unsigned long)start,
                       (unsigned long)stop,
                       size);
                total_used += size;
            }

            ptr += size;
        }

        printf("--------------------------------\n");
        printf("Total used memory : %zu bytes\n", total_used);
        printf("Total free memory : %zu bytes\n", total_free);
        return;
    }

    block_t *blocks = allocator_get_blocks();
    size_t   count  = allocator_get_block_count();
    uint8_t *base   = allocator_get_base();

    size_t total_used = 0;
    size_t total_free = 0;

    for (size_t i = 0; i < count; i++) {

        uintptr_t start = (uintptr_t)(base + blocks[i].offset);
        uintptr_t end   = start + blocks[i].size - 1;

        if (blocks[i].free) {
            printf("[0x%016lx - 0x%016lx] FREE (%zu bytes)\n",
                   (unsigned long)start,
                   (unsigned long)end,
                   blocks[i].size);
            total_free += blocks[i].size;
        } else {
            printf("[0x%016lx - 0x%016lx] USED (%zu bytes)\n",
                   (unsigned long)start,
                   (unsigned long)end,
                   blocks[i].size);
            total_used += blocks[i].size;
        }
    }

    printf("--------------------------------\n");
    printf("Total used memory : %zu bytes\n", total_used);
    printf("Total free memory : %zu bytes\n", total_free);
}
