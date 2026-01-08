#include "worst_fit.h"
#include "allocator.h"

/*
 * Worst-fit on out-of-band metadata
 * Returns index of largest suitable block, or -1
 */

int wf_find(size_t req) {
    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();

    int worst = -1;

    for (size_t i = 0; i < count; i++) {
        if (blocks[i].free && blocks[i].size >= req) {
            if (worst == -1 || blocks[i].size > blocks[worst].size) {
                worst = (int)i;
            }
        }
    }
    return worst;
}
