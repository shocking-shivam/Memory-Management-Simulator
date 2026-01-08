#ifndef FIRST_FIT_H
#define FIRST_FIT_H

#include <stddef.h>

/*
 * First-fit allocator (out-of-band metadata)
 * Returns index of first suitable free block, or -1
 */
int ff_find(size_t req);

#endif /* FIRST_FIT_H */
