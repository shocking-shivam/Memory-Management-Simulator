#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cli.h"
#include "cache.h"

#include "../allocator/allocator.h"
#include "../allocator/buddy.h"
#include "../stats/stats.h"

#define MAX_CLI_ALLOCS 1024

typedef struct {
    uint32_t id;
    int in_use;
} cli_alloc_t;

static cli_alloc_t alloc_table[MAX_CLI_ALLOCS];

/* =========================
   HELPERS
   ========================= */

static void print_prompt(void) {
    printf("> ");
    fflush(stdout);
}

static void reset_alloc_table(void) {
    for (int i = 0; i < MAX_CLI_ALLOCS; i++) {
        alloc_table[i].in_use = 0;
        alloc_table[i].id = 0;
    }
}

static int store_alloc(uint32_t id) {
    for (int i = 0; i < MAX_CLI_ALLOCS; i++) {
        if (!alloc_table[i].in_use) {
            alloc_table[i].in_use = 1;
            alloc_table[i].id = id;
            return 0;
        }
    }
    return -1;
}

static int has_alloc(uint32_t id) {
    for (int i = 0; i < MAX_CLI_ALLOCS; i++) {
        if (alloc_table[i].in_use && alloc_table[i].id == id)
            return 1;
    }
    return 0;
}

static void clear_alloc(uint32_t id) {
    for (int i = 0; i < MAX_CLI_ALLOCS; i++) {
        if (alloc_table[i].in_use && alloc_table[i].id == id) {
            alloc_table[i].in_use = 0;
            alloc_table[i].id = 0;
            return;
        }
    }
}

/* Resolve allocation ID → payload base address */
static void *resolve_address(uint32_t id) {
    if (get_allocator_algo() == ALGO_BUDDY) {
        return buddy_allocated_address(id);
    }

    block_t *blocks = allocator_get_blocks();
    size_t count = allocator_get_block_count();
    uint8_t *base = allocator_get_base();

    for (size_t i = 0; i < count; i++) {
        if (!blocks[i].free && blocks[i].id == id) {
            return base + blocks[i].offset;
        }
    }

    return NULL;
}

/* =========================
   MAIN CLI LOOP
   ========================= */

void cli_run(void) {
    char line[256];

    while (1) {
        print_prompt();

        if (!fgets(line, sizeof(line), stdin))
            break;

        char *cmd = strtok(line, " \n");
        if (!cmd)
            continue;

        /* help */
        if (strcmp(cmd, "help") == 0) {
            printf("Commands:\n");
            printf("  init memory <size>\n");
            printf("  set allocator <first|best|worst|buddy>\n");
            printf("  malloc <size>\n");
            printf("  free <id>\n");
            printf("  read <id> <offset>\n");
            printf("  write <id> <offset>\n");
            printf("  dump\n");
            printf("  stats\n");
            printf("  cache_stats\n");
            printf("  shutdown\n");
            printf("  exit | quit\n");
        }

        /* init */
        else if (strcmp(cmd, "init") == 0) {
            char *sub = strtok(NULL, " \n");
            char *sz  = strtok(NULL, " \n");

            if (!sub || !sz || strcmp(sub, "memory") != 0) {
                printf("Usage: init memory <size>\n");
                continue;
            }

            size_t size = (size_t)atoi(sz);
            if (mem_init(size) == 0) {
                reset_alloc_table();
                cache_init();
                printf("initialized memory: %zu bytes\n", size);
            }
        }

        /* set allocator */
        else if (strcmp(cmd, "set") == 0) {
            char *sub = strtok(NULL, " \n");
            char *arg = strtok(NULL, " \n");

            if (!sub || !arg || strcmp(sub, "allocator") != 0) {
                printf("Usage: set allocator <first|best|worst|buddy>\n");
                continue;
            }

            if (strcmp(arg, "first") == 0)
                set_allocator_algo(ALGO_FIRST_FIT);
            else if (strcmp(arg, "best") == 0)
                set_allocator_algo(ALGO_BEST_FIT);
            else if (strcmp(arg, "worst") == 0)
                set_allocator_algo(ALGO_WORST_FIT);
            else if (strcmp(arg, "buddy") == 0)
                set_allocator_algo(ALGO_BUDDY);
            else {
                printf("Unknown allocator strategy\n");
                continue;
            }

            printf("Allocator strategy set\n");
        }

        /* malloc */
        else if (strcmp(cmd, "malloc") == 0) {
            char *sz = strtok(NULL, " \n");
            if (!sz) {
                printf("Usage: malloc <size>\n");
                continue;
            }

            size_t size = (size_t)atoi(sz);
            uint32_t id = mem_alloc(size);

            if (!id) {
                printf("Allocation failed\n");
                continue;
            }

            store_alloc(id);

            void *addr = resolve_address(id);

            printf("Allocated block id=%u at address=0x%016lx\n",
                   id, (unsigned long)(uintptr_t)addr);
        }

        /* free */
        else if (strcmp(cmd, "free") == 0) {
            char *idstr = strtok(NULL, " \n");
            if (!idstr) {
                printf("Usage: free <id>\n");
                continue;
            }

            uint32_t id = (uint32_t)atoi(idstr);

            if (!has_alloc(id)) {
                printf("Invalid block id\n");
                continue;
            }

            mem_free(id);
            clear_alloc(id);
            printf("Block %u freed\n", id);
        }

        /* READ */
        else if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0) {
            int is_write = (strcmp(cmd, "write") == 0);

            char *idstr = strtok(NULL, " \n");
            char *offstr = strtok(NULL, " \n");

            if (!idstr || !offstr) {
                printf("Usage: %s <id> <offset>\n", cmd);
                continue;
            }

            uint32_t id = (uint32_t)atoi(idstr);
            size_t offset = (size_t)atoi(offstr);

            if (!has_alloc(id)) {
                printf("Invalid block id\n");
                continue;
            }

            void *base = resolve_address(id);
            if (!base) {
                printf("Failed to resolve address\n");
                continue;
            }

            uint64_t addr = (uint64_t)(uintptr_t)base + offset;
            cache_access(addr, is_write);

            printf("%s access at address 0x%016llx\n",
                   is_write ? "WRITE" : "READ",
                   (unsigned long long)addr);
        }

        /* dump */
        else if (strcmp(cmd, "dump") == 0) {
            mem_dump();
        }

        /* stats */
        else if (strcmp(cmd, "stats") == 0) {
            mem_stats_print();
        }

        /* cache stats */
        else if (strcmp(cmd, "cache_stats") == 0) {
            cache_report_stats();
        }

        /* shutdown */
        else if (strcmp(cmd, "shutdown") == 0) {
            mem_shutdown();
            cache_shutdown();
            reset_alloc_table();
            printf("Memory shutdown completed\n");
        }

        /* exit */
        else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }

        else {
            printf("Unknown command\n");
        }
    }
}
