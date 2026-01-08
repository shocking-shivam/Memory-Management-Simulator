#include "memory_dump.h"
#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdint.h>

void memory_dump(void) {
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();
    uint8_t *base = allocator_get_base();

    size_t total_used = 0;
    size_t total_free = 0;

    printf("\n========== MEMORY DUMP ==========\n");

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
