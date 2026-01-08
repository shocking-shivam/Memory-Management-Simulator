#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stddef.h>

/* =========================
   CACHE CONFIGURATION
   ========================= */

/* Cache access type */
#define CACHE_READ   0
#define CACHE_WRITE  1

/* Latencies (in cycles) */
#define L1_LATENCY   1
#define L2_LATENCY   5
#define L3_LATENCY   20
#define RAM_LATENCY  100

/* Replacement policies */
#define CACHE_LRU    0
#define CACHE_FIFO  1

/* =========================
   CACHE LIFECYCLE
   ========================= */

/* Initialize all cache levels (L1/L2/L3) */
void cache_init(void);

/* Free all cache memory */
void cache_shutdown(void);

/* =========================
   CACHE ACCESS (CORE API)
   ========================= */

/*
 * Simulate a CPU memory access.
 *
 * address  : physical memory address
 * is_write : CACHE_READ or CACHE_WRITE
 */
void cache_access(uint64_t address, int is_write);

/* =========================
   CACHE STATS / REPORTING
   ========================= */

/* Print cache statistics for all levels */
void cache_report_stats(void);

#endif /* CACHE_H */
