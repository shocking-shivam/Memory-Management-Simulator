#ifndef STATS_H
#define STATS_H

#include <stddef.h>

/* initialization */
void stats_init(size_t total_memory);
void stats_shutdown(void);

/* allocation tracking */
void stats_record_alloc_attempt(void);
void stats_record_alloc_success(void);
void stats_record_alloc_failure(void);
void stats_record_free(void);

/* USER MEMORY (what malloc() asked for) */
void stats_inc_user_used(size_t requested_bytes);
void stats_dec_user_used(size_t requested_bytes);

/* FRAGMENTATION (buddy-only) */
void stats_add_internal_frag(size_t bytes);
void stats_sub_internal_frag(size_t bytes);
void stats_set_external_frag(double value);

/* allocator overhead (metadata, optional, NOT added to used) */
void stats_set_allocator_overhead(size_t bytes);

/* cache tracking (optional) */
void stats_record_cache_hit(void);
void stats_record_cache_miss(void);

/* output */
void stats_print(void);

#endif
