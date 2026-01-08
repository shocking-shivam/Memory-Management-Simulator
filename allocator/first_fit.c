#include "first_fit.h"
#include "allocator.h"

/*
 * First-fit on out-of-band metadata
 * Returns index of first suitable block, or -1
 */

int ff_find(size_t req) {
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();

    for (size_t i = 0; i < count; i++) {
        if (blocks[i].free && blocks[i].size >= req)
            return (int)i;
    }
    return -1;
}
