#include "best_fit.h"
#include "allocator.h"

/*
 * Best-fit on out-of-band metadata
 * Returns index of best suitable block, or -1
 */

int bf_find(size_t req) {
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();

    int best = -1;

    for (size_t i = 0; i < count; i++) {
        if (blocks[i].free && blocks[i].size >= req) {
            if (best == -1 || blocks[i].size < blocks[best].size) {
                best = (int)i;
            }
        }
    }
    return best;
}
