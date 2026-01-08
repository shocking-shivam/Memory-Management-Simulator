#include "my_malloc.h"

#include "allocator/allocator.h"
#include "allocator/buddy.h"

#include <stdint.h>
#include <stddef.h>

/* ================================
   ID → POINTER (PUBLIC)
   ================================ */

void *id_to_ptr(uint32_t id) {
    if (id == 0)
        return NULL;

    /* Buddy allocator */
    if (get_allocator_algo() == ALGO_BUDDY) {
        return buddy_allocated_address(id);
    }

    /* First / Best / Worst Fit */
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();
    uint8_t *base = allocator_get_base();

    for (size_t i = 0; i < count; i++) {
        if (!blocks[i].free && blocks[i].id == id) {
            return base + blocks[i].offset;
        }
    }

    return NULL;
}

/* ================================
   POINTER → ID (LIMITED)
   ================================ */

static uint32_t ptr_to_id(void *ptr) {
    if (!ptr)
        return 0;

    /* Buddy allocator does NOT support reverse lookup safely */
    if (get_allocator_algo() == ALGO_BUDDY) {
        return 0;
    }

    /* First / Best / Worst Fit */
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();
    uint8_t *base = allocator_get_base();

    for (size_t i = 0; i < count; i++) {
        if (!blocks[i].free) {
            void *payload = base + blocks[i].offset;
            if (payload == ptr)
                return blocks[i].id;
        }
    }

    return 0;
}

/* ================================
   PUBLIC API
   ================================ */

void *my_malloc(size_t size) {
    uint32_t id = mem_alloc(size);
    if (id == 0)
        return NULL;

    return id_to_ptr(id);
}

void my_free(void *ptr) {
    if (!ptr)
        return;

    uint32_t id = ptr_to_id(ptr);
    if (id != 0)
        mem_free(id);
}
