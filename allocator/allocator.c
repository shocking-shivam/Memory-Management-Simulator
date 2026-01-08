#define _GNU_SOURCE
#include "allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../stats/stats.h"
#include "buddy.h"

/* ================= CONFIG ================= */

#define MAX_BLOCKS 1024

/* ================= GLOBALS ================= */

static uint8_t *mem_base = NULL;
static size_t   mem_total = 0;

static block_t  blocks[MAX_BLOCKS];
static size_t   block_count = 0;

static uint32_t next_id = 1;
static algo_t   current_algo = ALGO_FIRST_FIT;

/* ================= ACCESSORS ================= */

uint8_t *allocator_get_base(void) { return mem_base; }
size_t   allocator_get_total(void) { return mem_total; }

block_t *allocator_get_blocks(void) { return blocks; }
size_t   allocator_get_block_count(void) { return block_count; }

/* ================= INIT / SHUTDOWN ================= */

int mem_init(size_t bytes)
{
    if (mem_base) {
        fprintf(stderr, "memory already initialized\n");
        return -1;
    }

    mem_base = malloc(bytes);
    if (!mem_base)
        return -1;

    mem_total = bytes;

    blocks[0] = (block_t){
        .offset = 0,
        .size = bytes,
        .requested_size = 0,
        .free = true,
        .id = 0
    };
    block_count = 1;
    next_id = 1;

    stats_init(mem_total);

    if (current_algo == ALGO_BUDDY)
        buddy_init_pool(mem_base, mem_total);

    return 0;
}

void mem_shutdown(void)
{
    if (!mem_base)
        return;

    if (current_algo == ALGO_BUDDY)
        buddy_shutdown_pool();

    free(mem_base);
    mem_base = NULL;
    mem_total = 0;
    block_count = 0;

    stats_shutdown();
}

/* ================= CONFIG ================= */

void set_allocator_algo(algo_t a)
{
    if (a == current_algo)
        return;

    if (current_algo == ALGO_BUDDY)
        buddy_shutdown_pool();

    current_algo = a;

    if (current_algo == ALGO_BUDDY && mem_base)
        buddy_init_pool(mem_base, mem_total);
}

algo_t get_allocator_algo(void)
{
    return current_algo;
}

/* ================= INTERNAL HELPERS ================= */

static int find_block(size_t bytes)
{
    int best = -1;

    for (size_t i = 0; i < block_count; i++) {
        if (!blocks[i].free || blocks[i].size < bytes)
            continue;

        if (current_algo == ALGO_FIRST_FIT)
            return (int)i;

        if (best == -1)
            best = (int)i;
        else if (current_algo == ALGO_BEST_FIT &&
                 blocks[i].size < blocks[best].size)
            best = (int)i;
        else if (current_algo == ALGO_WORST_FIT &&
                 blocks[i].size > blocks[best].size)
            best = (int)i;
    }
    return best;
}

/* ================= ALLOC ================= */

uint32_t mem_alloc(size_t bytes)
{
    if (!mem_base || bytes == 0)
        return 0;

    stats_record_alloc_attempt();

    if (current_algo == ALGO_BUDDY) {
        uint32_t id = buddy_alloc(bytes);
        if (id)
            stats_record_alloc_success();
        else
            stats_record_alloc_failure();
        return id;
    }

    int idx = find_block(bytes);
    if (idx < 0) {
        stats_record_alloc_failure();
        return 0;
    }

    block_t *b = &blocks[idx];

    if (b->size > bytes) {
        memmove(&blocks[idx + 2], &blocks[idx + 1],
                (block_count - idx - 1) * sizeof(block_t));

        blocks[idx + 1] = (block_t){
            .offset = b->offset + bytes,
            .size = b->size - bytes,
            .requested_size = 0,
            .free = true,
            .id = 0
        };
        block_count++;
        b->size = bytes;
    }

    b->free = false;
    b->id = next_id++;
    b->requested_size = bytes;

    stats_record_alloc_success();
    return b->id;
}

/* ================= FREE ================= */

int mem_free(uint32_t id)
{
    if (current_algo == ALGO_BUDDY) {
        int r = buddy_free(id);
        if (r == 0)
            stats_record_free();
        return r;
    }

    for (size_t i = 0; i < block_count; i++) {
        if (!blocks[i].free && blocks[i].id == id) {

            blocks[i].free = true;
            blocks[i].id = 0;
            blocks[i].requested_size = 0;

            stats_record_free();

            if (i + 1 < block_count && blocks[i + 1].free) {
                blocks[i].size += blocks[i + 1].size;
                memmove(&blocks[i + 1], &blocks[i + 2],
                        (block_count - i - 2) * sizeof(block_t));
                block_count--;
            }

            if (i > 0 && blocks[i - 1].free) {
                blocks[i - 1].size += blocks[i].size;
                memmove(&blocks[i], &blocks[i + 1],
                        (block_count - i - 1) * sizeof(block_t));
                block_count--;
            }

            return 0;
        }
    }
    return -1;
}

/* ================= DUMP / STATS ================= */

void mem_dump(void)
{
    extern void memory_dump(void);
    memory_dump();
}

void mem_stats_print(void)
{
    stats_print();
}
