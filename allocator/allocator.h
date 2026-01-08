#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ================= ALLOCATOR POLICY ================= */

typedef enum {
    ALGO_FIRST_FIT = 0,
    ALGO_BEST_FIT,
    ALGO_WORST_FIT,
    ALGO_BUDDY
} algo_t;

/* ================= OUT-OF-BAND METADATA ================= */

/*
 * IMPORTANT DESIGN RULE:
 * - Heap contains ONLY user payload
 * - No headers or metadata inside heap
 * - All block information is stored out-of-band
 *
 * Internal fragmentation formula (universal):
 *   size - requested_size
 */
typedef struct {
    size_t offset;          /* offset into heap */
    size_t size;            /* actual allocated size */
    size_t requested_size;  /* bytes requested by user */
    bool   free;            /* free or allocated */
    uint32_t id;            /* allocation id */
} block_t;

/* ================= CORE API ================= */

int mem_init(size_t bytes);
void mem_shutdown(void);

void set_allocator_algo(algo_t a);
algo_t get_allocator_algo(void);

int mem_set_cache(int on);
int mem_get_cache(void);

uint32_t mem_alloc(size_t bytes);
int mem_free(uint32_t alloc_id);

/* ================= OBSERVABILITY ================= */

void mem_dump(void);
void mem_stats_print(void);

/* ================= HEAP ACCESS ================= */

uint8_t *allocator_get_base(void);
size_t   allocator_get_total(void);

/* ================= METADATA ACCESS ================= */

/* Used by dump, stats, and fit algorithms */
block_t *allocator_get_blocks(void);
size_t   allocator_get_block_count(void);

#endif /* ALLOCATOR_H */
