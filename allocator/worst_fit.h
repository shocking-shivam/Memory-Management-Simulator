#ifndef WORST_FIT_H
#define WORST_FIT_H

#include <stddef.h>

/*
 * Worst-fit allocator (out-of-band metadata)
 * Returns index of largest suitable free block, or -1
 */
int wf_find(size_t req);

#endif /* WORST_FIT_H */
