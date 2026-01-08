#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>
#include <stddef.h>

/* lifecycle */
int  buddy_init_pool(uint8_t *base, size_t bytes);
void buddy_shutdown_pool(void);

/* allocation */
uint32_t buddy_alloc(size_t bytes);
int      buddy_free(uint32_t id);

/* helpers for CLI & stats */
void    *buddy_allocated_address(uint32_t id);
size_t   buddy_allocated_size(uint32_t id);

/* read-only accessors */
uint8_t *buddy_get_base(void);
size_t   buddy_get_size(void);

#endif /* BUDDY_H */
