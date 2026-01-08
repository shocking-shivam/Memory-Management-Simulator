/* Optional small arena helper (not required by allocator which uses malloc).
   Provided for completeness in the project. */

#include <stdlib.h>
#include <stdint.h>

static uint8_t *arena = NULL;
static size_t arena_size = 0;
static size_t arena_offset = 0;

int my_sbrk_init(size_t sz) {
    if (arena) return -1;
    arena = (uint8_t*)malloc(sz);
    if (!arena) return -1;
    arena_size = sz;
    arena_offset = 0;
    return 0;
}

void *my_sbrk(size_t inc) {
    if (!arena) return NULL;
    if (arena_offset + inc > arena_size) return NULL;
    void *p = arena + arena_offset;
    arena_offset += inc;
    return p;
}

void my_sbrk_shutdown(void) {
    if (arena) free(arena);
    arena = NULL; arena_size = 0; arena_offset = 0;
}
