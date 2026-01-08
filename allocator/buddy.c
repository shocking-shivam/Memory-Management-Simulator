#define _GNU_SOURCE
#include "buddy.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Buddy allocator configuration */
#define MIN_ORDER 5    /* 32 bytes */
#define MAX_ORDER 22   /* up to 4MB */

/* Free list node */
typedef struct bnode {
    struct bnode *next;
} bnode_t;

/* Header stored at start of every block */
typedef struct {
    uint32_t id;              /* 0 if free */
    uint32_t order;           /* block size = 2^order */
    uint32_t requested_size;  /* user requested bytes */
} buddy_hdr_t;

/* Globals */
static uint8_t  *buddy_base = NULL;
static size_t    buddy_size = 0;
static int       max_order_local = MAX_ORDER;
static bnode_t **free_lists = NULL;
static uint32_t  buddy_next_id = 1;

/* Determine minimum order for requested size */
static int order_for_size(size_t bytes)
{
    int order = MIN_ORDER;
    size_t sz = 1UL << order;

    while (sz < bytes && order <= max_order_local) {
        order++;
        sz <<= 1;
    }
    return (order > max_order_local) ? -1 : order;
}

/* Initialize buddy memory pool */
int buddy_init_pool(uint8_t *base, size_t bytes)
{
    if (free_lists) {
        free(free_lists);
        free_lists = NULL;
    }

    buddy_base = base;
    buddy_size = bytes;

    int o = MIN_ORDER;
    while ((1UL << (o + 1)) <= buddy_size && o < MAX_ORDER)
        o++;

    max_order_local = o;

    free_lists = calloc(max_order_local + 1, sizeof(bnode_t *));
    if (!free_lists)
        return -1;

    buddy_hdr_t *root = (buddy_hdr_t *)buddy_base;
    root->id = 0;
    root->order = max_order_local;
    root->requested_size = 0;

    free_lists[max_order_local] = (bnode_t *)buddy_base;
    free_lists[max_order_local]->next = NULL;

    buddy_next_id = 1;
    return 0;
}

/* Shutdown buddy pool */
void buddy_shutdown_pool(void)
{
    if (free_lists) {
        free(free_lists);
        free_lists = NULL;
    }
    buddy_base = NULL;
    buddy_size = 0;
}

/* Pop block from free list */
static bnode_t *pop_block(int order)
{
    bnode_t *b = free_lists[order];
    if (!b)
        return NULL;
    free_lists[order] = b->next;
    b->next = NULL;
    return b;
}

/* Push block into free list */
static void push_block(int order, bnode_t *b)
{
    b->next = free_lists[order];
    free_lists[order] = b;
}

/* Split block into two buddies */
static void split_block(int from)
{
    bnode_t *b = pop_block(from);
    if (!b)
        return;

    size_t sz = 1UL << from;
    uint8_t *addr = (uint8_t *)b;

    bnode_t *left  = (bnode_t *)addr;
    bnode_t *right = (bnode_t *)(addr + (sz >> 1));

    buddy_hdr_t *hl = (buddy_hdr_t *)left;
    buddy_hdr_t *hr = (buddy_hdr_t *)right;

    hl->id = hr->id = 0;
    hl->requested_size = hr->requested_size = 0;
    hl->order = hr->order = from - 1;

    push_block(from - 1, right);
    push_block(from - 1, left);
}

/* Allocate memory */
uint32_t buddy_alloc(size_t bytes)
{
    if (!buddy_base || bytes == 0)
        return 0;

    int want = order_for_size(bytes);
    if (want < 0)
        return 0;

    int i = want;
    while (i <= max_order_local && !free_lists[i])
        i++;
    if (i > max_order_local)
        return 0;

    while (i > want) {
        split_block(i);
        i--;
    }

    bnode_t *b = pop_block(want);
    if (!b)
        return 0;

    buddy_hdr_t *hdr = (buddy_hdr_t *)b;
    hdr->id = buddy_next_id++;
    hdr->order = want;
    hdr->requested_size = bytes;

    return hdr->id;
}

/* Free allocated block */
int buddy_free(uint32_t id)
{
    if (!buddy_base)
        return -1;

    uintptr_t cur = (uintptr_t)buddy_base;
    uintptr_t end = cur + buddy_size;

    while (cur < end) {
        buddy_hdr_t *hdr = (buddy_hdr_t *)cur;
        size_t block_size = 1UL << hdr->order;

        if (hdr->id == id) {
            hdr->id = 0;
            hdr->requested_size = 0;
            push_block(hdr->order, (bnode_t *)hdr);
            return 0;
        }
        cur += block_size;
    }
    return -1;
}

/* Return payload address for allocation ID */
void *buddy_allocated_address(uint32_t id)
{
    uintptr_t cur = (uintptr_t)buddy_base;
    uintptr_t end = cur + buddy_size;

    while (cur < end) {
        buddy_hdr_t *hdr = (buddy_hdr_t *)cur;
        size_t block_size = 1UL << hdr->order;

        if (hdr->id == id)
            return (void *)(cur + sizeof(buddy_hdr_t));

        cur += block_size;
    }
    return NULL;
}

/* Return requested allocation size */
size_t buddy_allocated_size(uint32_t id)
{
    uintptr_t cur = (uintptr_t)buddy_base;
    uintptr_t end = cur + buddy_size;

    while (cur < end) {
        buddy_hdr_t *hdr = (buddy_hdr_t *)cur;
        size_t block_size = 1UL << hdr->order;

        if (hdr->id == id)
            return hdr->requested_size;

        cur += block_size;
    }
    return 0;
}

/* Read-only accessors for dump / stats */
uint8_t *buddy_get_base(void) { return buddy_base; }
size_t   buddy_get_size(void) { return buddy_size; }
