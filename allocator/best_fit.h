#ifndef BEST_FIT_H
#define BEST_FIT_H

#include <stddef.h>

/*
 * Best-fit allocator (out-of-band metadata)
 * Returns index of best suitable free block, or -1
 */
int bf_find(size_t req);

#endif /* BEST_FIT_H */
