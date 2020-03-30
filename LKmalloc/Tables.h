#ifndef TABLES_H
#define TABLES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "Flags.h"

/**
 * Data structure for holding a record of a malloc.
 */
typedef struct MALLOCS_TABLE
{
    struct MALLOCS_TABLE *next;
    void *malloc_ptr;
    void *orig_ptr;
    uint64_t malloc_size;
    uint16_t malloc_flags;
    uint8_t is_used;
    int timestamp;
    unsigned long microseconds;
    int line;
    char *file;
    char *func;
    // below are variables only used by the first (main) mallocs table
    uint64_t total_mallocs;
} MALLOCS_TABLE;

/**
 * Data structure for holding a record of a free.
 */
typedef struct FREES_TABLE
{
    struct FREES_TABLE *next;
    void *free_ptr;
    void *orig_ptr;
    void *approx_free_ptr;
    uint64_t free_size;
    uint16_t free_flags;
    uint8_t is_used;
    int timestamp;
    unsigned long microseconds;
    int line;
    char *file;
    char *func;
    // below are variables only used by the first (main) frees table
    uint64_t total_frees;
} FREES_TABLE;

#endif
