#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "Tables.c"
#include "Flags.h"

#define lkmalloc(size, ptr, flags) __lkmalloc_internal((size), (ptr), (flags), __FILE__, __func__, __LINE__)

#define lkfree(ptr, flags) __lkfree_internal((ptr), (flags), __FILE__, __func__, __LINE__, NULL, (flags))

/**
 * This will ask to allocate "size" bytes, and if successful, assign the newly allocated address to *ptr. 
 * By passing an addr or a ptr to lkmalloc(), you can do more intelligent checking of bugs. 
 * lkmalloc() should return 0 on success -errno on failure (e.g., -ENOMEM).
 */
int __lkmalloc_internal(uint64_t size, void **ptr, uint16_t flags, const char *file, const char *func, int line)
{
    errno = 0;

    // simply allocate only the given amt
    if (LKM_REG == flags)
    {
        void *new_ptr;
        if ((new_ptr = malloc(size)) == NULL)
        {
            return -errno;
        }

        MALLOCS_TABLE *table = _lk_get_empty_malloc_table(main_mallocs_table);
        if (table == NULL)
        {
            fprintf(stderr, "error: unable to get an empty table\n");
            return -ENOTRECOVERABLE;
        }

        table->is_used = 1;
        table->malloc_flags = flags;
        table->orig_ptr = *ptr;
        table->malloc_ptr = new_ptr;
        table->malloc_size = size;
        table->timestamp = (int)time(NULL);
        // a more accurate timestamp
        struct timeval tv;
        gettimeofday(&tv, NULL);
        table->microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
        // malloc and cpy over the file and func and line it was from
        table->line = line;
        table->file = malloc(strlen(file) + 1);
        strcpy(table->file, file);
        table->func = malloc(strlen(func) + 1);
        strcpy(table->func, func);

        // increment the number of malloc's we've done.
        main_mallocs_table->total_mallocs++;

        // malloc was successful, so set *ptr to the malloc'd ptr
        *ptr = new_ptr;

        return 0;
    }

    if (flags & LKM_INIT)
    {
        // simply do a regular alloc, then memset to 0's.
        int res = 0;
        if ((res = __lkmalloc_internal(size, ptr, (flags & ~LKM_INIT) | LKM_REG, file, func, line)) < 0)
        {
            return res;
        }

        // now memset to 0's
        if (memset(*ptr, 0, size) == NULL)
        {
            return -errno;
        }

        return 0;
    }

    return -errno;
}

/**
 * This'll take the addr of the ptr that was presumably allocated by lkmalloc (but maybe not), and attempt to free it. 
 * Return 0 on success, -errno on failure (e.g., -EINVAL, etc.).
 */
int __lkfree_internal(void **ptr, uint16_t flags, const char *file, const char *func, int line, void **original, uint16_t rflags)
{
    errno = 0;

    if (*ptr == NULL)
    {
        return -EINVAL;
    }

    if (flags & LKF_APPROX)
    {
        MALLOCS_TABLE *table;
        if ((table = _lk_find_malloc_table_with_ptr_near(ptr, flags)) != NULL)
        {
            return __lkfree_internal(&(table->malloc_ptr), (flags & ~LKF_APPROX) | LKF_REG, file, func, line, ptr, rflags);
        }

        return __lkfree_internal(ptr, (flags & ~LKF_APPROX) | LKF_REG, file, func, line, ptr, rflags);
    }

    if ((flags & LKF_REG) == 0)
    {
        FREES_TABLE *table = _lk_get_empty_free_table(main_frees_table);
        if (table == NULL)
        {
            return -ENOTRECOVERABLE;
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);
        if (_lk_find_malloc_table_with_ptr_before_time(*ptr, 1000000 * tv.tv_sec + tv.tv_usec) == NULL)
        {
            if (flags & LKF_UNKNOWN)
            {
                fprintf(stderr, "LKF_UNKNOWN: Warning - given pointer '%p' was never allocated.\n", *ptr);
            }

            if (flags & LKF_ERROR)
            {
                fprintf(stderr, "LKF_ERROR: exiting program.\n");
                exit(-LKF_ERROR); // not sure what # to exit with so might as-well.
            }
        }

        table->is_used = 1;
        table->free_flags = rflags;
        table->orig_ptr = *ptr;
        table->free_ptr = *ptr;
        table->free_size = 0; // how could we determine the size to free?
        table->timestamp = (int)time(NULL);
        // a more accurate timestamp
        gettimeofday(&tv, NULL);
        table->microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
        // malloc and cpy over the file and func and line it was from
        table->line = line;
        table->file = malloc(strlen(file) + 1);
        strcpy(table->file, file);
        table->func = malloc(strlen(func) + 1);
        strcpy(table->func, func);
        // set it's original approximate value
        table->approx_free_ptr = NULL;
        if (original != NULL)
        {
            if (*original != NULL)
            {
                table->approx_free_ptr = *original;
            }
        }

        // increment the number of malloc's we've done.
        main_frees_table->total_frees++;

        if (_lk_find_malloc_table_with_ptr_near(ptr, ((flags & ~LKF_UNKNOWN) & ~LKF_WARN)) != NULL && _lk_find_free_table_with_ptr(ptr) == NULL)
        {
            free(*ptr);
        }

        return -errno;
    }

    return -errno;
}

/**
 * Print out to the specified fd a report of the specified nature by the given flags.
 */
int lkreport(int fd, uint16_t flags)
{
    errno = 0;

    FILE *fp = fdopen(fd, "w");
    if (fp == NULL)
    {
        return -errno;
    }

    /**
     * Write the headers of the CSV initially
     */
    write_csv_header(fp);

    // print nothing
    if (flags == LKR_NONE)
    {
        return 0;
    }

    // memory leaks
    if (flags & LKR_SERIOUS)
    {
        _lk_print_malloc_tables_with_no_frees(fp);

        return lkreport(fd, (flags & ~LKR_SERIOUS) | LKR_NONE);
    }

    // matching pairs
    if (flags & LKR_MATCH)
    {
        _lk_print_malloc_free_matches(fp);

        return lkreport(fd, (flags & ~LKR_MATCH) | LKR_NONE);
    }

    // bad free - passed addr in middle of alloc addr
    if (flags & LKR_BAD_FREE)
    {
        _lk_print_bad_frees(fp);

        return lkreport(fd, (flags & ~LKR_BAD_FREE) | LKR_NONE);
    }

    // orphan free - free called on value that was never allocated
    if (flags & LKR_ORPHAN_FREE)
    {
        _lk_print_orphan_frees(fp);

        return lkreport(fd, (flags & ~LKR_ORPHAN_FREE) | LKR_NONE);
    }

    // double free - free called on a value twice
    if (flags & LKR_DOUBLE_FREE)
    {
        _lk_print_double_frees(fp);

        return lkreport(fd, (flags & ~LKR_DOUBLE_FREE) | LKR_NONE);
    }

    // approx match - show free's that match with mallocs; where the free was an approx match, not exact.
    if (flags & LKR_APPROX)
    {
        _lk_print_approx_matches(fp);

        return lkreport(fd, (flags & ~LKR_APPROX) | LKR_NONE);
    }

    return -errno;
}

/**
 * Report wrapper - used for atexit().
 */
void lkreport_wrapper()
{
    lkreport(fileno(stderr), LKR_SERIOUS | LKR_BAD_FREE);
}
