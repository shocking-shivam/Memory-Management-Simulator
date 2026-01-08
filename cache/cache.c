#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================
   INTERNAL STRUCTURES
   ========================= */

typedef struct {
    uint64_t tag;
    int valid;
    int dirty;
    uint64_t insertion_time;
    uint64_t lru_time;
} cache_line_t;

typedef struct {
    cache_line_t *lines;
} cache_set_t;

typedef struct {
    char name[4];                 /* L1 / L2 / L3 */
    size_t size;                  /* total bytes */
    size_t block_size;            /* bytes per block */
    int associativity;
    int num_sets;
    int policy;                   /* CACHE_LRU / CACHE_FIFO */

    cache_set_t *sets;

    uint64_t hits;
    uint64_t misses;
    uint64_t global_time;
} cache_level_t;

typedef struct {
    cache_level_t l1;
    cache_level_t l2;
    cache_level_t l3;

    uint64_t total_requests;
    uint64_t total_cycles;
} cache_controller_t;

/* =========================
   GLOBAL CONTROLLER
   ========================= */

static cache_controller_t cache;

/* =========================
   HELPERS
   ========================= */

static void cache_level_init(
    cache_level_t *lvl,
    const char *name,
    size_t size,
    size_t block_size,
    int assoc,
    int policy
) {
    strncpy(lvl->name, name, sizeof(lvl->name));
    lvl->size = size;
    lvl->block_size = block_size;
    lvl->associativity = assoc;
    lvl->policy = policy;

    lvl->num_sets = (int)(size / (block_size * assoc));
    lvl->sets = (cache_set_t *)malloc(sizeof(cache_set_t) * lvl->num_sets);

    for (int i = 0; i < lvl->num_sets; i++) {
        lvl->sets[i].lines =
            (cache_line_t *)calloc(assoc, sizeof(cache_line_t));
    }

    lvl->hits = 0;
    lvl->misses = 0;
    lvl->global_time = 0;
}

static void cache_level_free(cache_level_t *lvl) {
    for (int i = 0; i < lvl->num_sets; i++) {
        free(lvl->sets[i].lines);
    }
    free(lvl->sets);
}

static int cache_level_access(cache_level_t *lvl, uint64_t address, int is_write) {
    lvl->global_time++;

    uint64_t block_addr = address / lvl->block_size;
    uint64_t set_index = block_addr % lvl->num_sets;
    uint64_t tag = block_addr / lvl->num_sets;

    cache_set_t *set = &lvl->sets[set_index];

    /* HIT CHECK */
    for (int i = 0; i < lvl->associativity; i++) {
        cache_line_t *line = &set->lines[i];
        if (line->valid && line->tag == tag) {
            lvl->hits++;
            if (lvl->policy == CACHE_LRU)
                line->lru_time = lvl->global_time;
            if (is_write)
                line->dirty = 1;
            return 1; /* HIT */
        }
    }

    /* MISS */
    lvl->misses++;

    /* EMPTY SLOT */
    for (int i = 0; i < lvl->associativity; i++) {
        cache_line_t *line = &set->lines[i];
        if (!line->valid) {
            line->valid = 1;
            line->tag = tag;
            line->dirty = is_write;
            line->insertion_time = lvl->global_time;
            line->lru_time = lvl->global_time;
            return 0;
        }
    }

    /* EVICTION */
    int victim = 0;
    uint64_t min_time = UINT64_MAX;

    for (int i = 0; i < lvl->associativity; i++) {
        cache_line_t *line = &set->lines[i];
        uint64_t t = (lvl->policy == CACHE_FIFO)
                         ? line->insertion_time
                         : line->lru_time;
        if (t < min_time) {
            min_time = t;
            victim = i;
        }
    }

    set->lines[victim].tag = tag;
    set->lines[victim].dirty = is_write;
    set->lines[victim].insertion_time = lvl->global_time;
    set->lines[victim].lru_time = lvl->global_time;

    return 0;
}

/* =========================
   PUBLIC API
   ========================= */

void cache_init(void) {
    memset(&cache, 0, sizeof(cache));

    cache_level_init(&cache.l1, "L1", 1024, 64, 2, CACHE_LRU);
    cache_level_init(&cache.l2, "L2", 4096, 64, 4, CACHE_LRU);
    cache_level_init(&cache.l3, "L3", 16384, 64, 8, CACHE_FIFO);
}

void cache_shutdown(void) {
    cache_level_free(&cache.l1);
    cache_level_free(&cache.l2);
    cache_level_free(&cache.l3);
}

void cache_access(uint64_t address, int is_write) {
    cache.total_requests++;

    printf("\nCPU %s Request: 0x%llx\n",
           is_write ? "WRITE" : "READ",
           (unsigned long long)address);

    uint64_t cost = L1_LATENCY;

    if (cache_level_access(&cache.l1, address, is_write)) {
        printf("-> L1 Hit (Cost: %llu cycles)\n",
               (unsigned long long)cost);
        cache.total_cycles += cost;
        return;
    }
    printf("-> L1 Miss\n");

    cost += L2_LATENCY;
    if (cache_level_access(&cache.l2, address, is_write)) {
        printf("-> L2 Hit (Cost: %llu cycles)\n",
               (unsigned long long)cost);
        cache.total_cycles += cost;
        return;
    }
    printf("-> L2 Miss\n");

    cost += L3_LATENCY;
    if (cache_level_access(&cache.l3, address, is_write)) {
        printf("-> L3 Hit (Cost: %llu cycles)\n",
               (unsigned long long)cost);
        cache.total_cycles += cost;
        return;
    }

    printf("-> L3 Miss (Accessing Main Memory)\n");

    cost += RAM_LATENCY;
    printf("-> Main Memory Access (Total Cost: %llu cycles)\n",
           (unsigned long long)cost);

    cache.total_cycles += cost;
}

void cache_report_stats(void) {
    printf("\n========== CACHE STATS ==========\n");

    cache_level_t *levels[3] = {&cache.l1, &cache.l2, &cache.l3};

    for (int i = 0; i < 3; i++) {
        cache_level_t *lvl = levels[i];
        uint64_t total = lvl->hits + lvl->misses;
        double rate = total ? (double)lvl->hits / total * 100.0 : 0.0;

        printf("[%s] Hits: %llu  Misses: %llu  HitRate: %.2f%%\n",
               lvl->name,
               (unsigned long long)lvl->hits,
               (unsigned long long)lvl->misses,
               rate);
    }

    printf("---------------------------------\n");
    printf("Total Requests : %llu\n", (unsigned long long)cache.total_requests);
    printf("Total Cycles   : %llu\n", (unsigned long long)cache.total_cycles);

    if (cache.total_requests > 0) {
        double amat =
            (double)cache.total_cycles / cache.total_requests;
        printf("AMAT           : %.2f cycles\n", amat);
    }

    printf("=================================\n");
}
