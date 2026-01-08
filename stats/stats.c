/* stats/stats.c */
#include "stats.h"
#include <stdio.h>
#include <stdint.h>

#include "../allocator/allocator.h"
#include "../allocator/buddy.h"

/* Allocation counters */
static size_t alloc_requests = 0;
static size_t alloc_success  = 0;
static size_t alloc_fail     = 0;
static size_t alloc_free     = 0;

/* Heap total size */
static size_t total_memory = 0;

/* =========================
   INIT / SHUTDOWN
   ========================= */

void stats_init(size_t total) {
    total_memory = total;
    alloc_requests = alloc_success = alloc_fail = alloc_free = 0;
}

void stats_shutdown(void) {}

/* =========================
   COUNTERS
   ========================= */

void stats_record_alloc_attempt(void) { alloc_requests++; }
void stats_record_alloc_success(void) { alloc_success++; }
void stats_record_alloc_failure(void) { alloc_fail++; }
void stats_record_free(void)          { alloc_free++; }

/* unused hooks (kept for API stability) */
void stats_inc_used(size_t bytes)      { (void)bytes; }
void stats_dec_used(size_t bytes)      { (void)bytes; }
void stats_add_internal_frag(size_t b) { (void)b; }
void stats_set_external_frag(double v) { (void)v; }
void stats_record_cache_hit(void)      {}
void stats_record_cache_miss(void)     {}

/* =========================
   PRINT STATS
   ========================= */

void stats_print(void) {

    /* =========================
       BUDDY ALLOCATOR STATS
       ========================= */
    if (get_allocator_algo() == ALGO_BUDDY) {

        size_t used_memory = 0;
        size_t free_memory = 0;
        size_t used_blocks = 0;
        size_t free_blocks = 0;
        size_t internal_frag = 0;
        size_t largest_free = 0;

        uintptr_t cur = (uintptr_t)buddy_get_base();
        uintptr_t end = cur + buddy_get_size();

        while (cur < end) {
            uint32_t id    = *(uint32_t*)cur;
            uint32_t order = *(uint32_t*)(cur + sizeof(uint32_t));
            size_t block_size = (size_t)1 << order;

            if (id != 0) {
                used_blocks++;

                size_t req = buddy_allocated_size(id);
                used_memory += req;

                if (req < block_size)
                    internal_frag += (block_size - req);
            } else {
                free_blocks++;
                free_memory += block_size;
                if (block_size > largest_free)
                    largest_free = block_size;
            }

            cur += block_size;
        }

        double utilization =
            total_memory ? (100.0 * used_memory / total_memory) : 0.0;

        double external_frag =
            (free_memory > 0)
                ? 1.0 - ((double)largest_free / free_memory)
                : 0.0;

        double success_rate =
            (alloc_requests > 0)
                ? (100.0 * alloc_success / alloc_requests)
                : 0.0;

        printf("\n---------- SUMMARY ----------\n");
        printf("Total heap size        : %zu bytes\n", total_memory);
        printf("Used memory            : %zu bytes\n", used_memory);
        printf("Free memory            : %zu bytes\n", free_memory);
        printf("Used blocks            : %zu\n", used_blocks);
        printf("Free blocks            : %zu\n", free_blocks);
        printf("Internal fragmentation : %zu bytes\n", internal_frag);
        printf("Memory utilization     : %.2f%%\n", utilization);
        printf("External fragmentation : %.3f\n", external_frag);
        printf("Allocation requests    : %zu\n", alloc_requests);
        printf("Successful allocs      : %zu\n", alloc_success);
        printf("Failed allocs          : %zu\n", alloc_fail);
        printf("Frees                  : %zu\n", alloc_free);
        printf("Success rate           : %.2f%%\n", success_rate);
        printf("-----------------------------\n");

        return;
    }

    /* =========================
       OUT-OF-BAND (FIRST/BEST/WORST)
       ========================= */

    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();

    size_t used_memory = 0;
    size_t free_memory = 0;
    size_t used_blocks = 0;
    size_t free_blocks = 0;
    size_t largest_free = 0;

    for (size_t i = 0; i < count; i++) {
        if (blocks[i].free) {
            free_blocks++;
            free_memory += blocks[i].size;
            if (blocks[i].size > largest_free)
                largest_free = blocks[i].size;
        } else {
            used_blocks++;
            used_memory += blocks[i].size;
        }
    }

    double utilization =
        total_memory ? (100.0 * used_memory / total_memory) : 0.0;

    double external_frag =
        (free_memory > 0)
            ? 1.0 - ((double)largest_free / free_memory)
            : 0.0;

    double success_rate =
        (alloc_requests > 0)
            ? (100.0 * alloc_success / alloc_requests)
            : 0.0;

    printf("\n---------- SUMMARY ----------\n");
    printf("Total heap size        : %zu bytes\n", total_memory);
    printf("Used memory            : %zu bytes\n", used_memory);
    printf("Free memory            : %zu bytes\n", free_memory);
    printf("Used blocks            : %zu\n", used_blocks);
    printf("Free blocks            : %zu\n", free_blocks);
    printf("Internal fragmentation : 0 bytes\n");
    printf("Memory utilization     : %.2f%%\n", utilization);
    printf("External fragmentation : %.3f\n", external_frag);
    printf("Allocation requests    : %zu\n", alloc_requests);
    printf("Successful allocs      : %zu\n", alloc_success);
    printf("Failed allocs          : %zu\n", alloc_fail);
    printf("Frees                  : %zu\n", alloc_free);
    printf("Success rate           : %.2f%%\n", success_rate);
    printf("-----------------------------\n");
}
