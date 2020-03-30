#include "Tables.h"

/**
 * Pointers to the initial malloc and free records...
 */
static MALLOCS_TABLE *main_mallocs_table;
static FREES_TABLE *main_frees_table;

/**
 * Keep track of whether we wrote the headers already.
 */
static int wrote_csv_header = 0;

/**
 * Returns a new empty entry for the data structure.
 * For getting an empty entry in the MALLOCs Table
 * 
 * Given an existing, filled table.
 * Should basically always be given first table.
 */
MALLOCS_TABLE *_lk_get_empty_malloc_table(MALLOCS_TABLE *main_table)
{
    if (main_mallocs_table == NULL)
    {
        // fprintf(stderr, "debug: main malloc table was null, creating first time table\n");
        main_mallocs_table = malloc(sizeof(MALLOCS_TABLE));
        main_mallocs_table->next = NULL;
        main_mallocs_table->malloc_ptr = NULL;
        main_mallocs_table->orig_ptr = NULL;
        main_mallocs_table->malloc_size = 0;
        main_mallocs_table->malloc_flags = 0;
        main_mallocs_table->is_used = 0;
        main_mallocs_table->total_mallocs = 0; // originally 0
        return main_mallocs_table;
    }

    if (main_table->next == NULL)
    {
        // fprintf(stderr, "debug: creating a next malloc table and returning it\n");
        MALLOCS_TABLE *new_table = malloc(sizeof(MALLOCS_TABLE));
        new_table->next = NULL;
        new_table->malloc_ptr = NULL;
        new_table->orig_ptr = NULL;
        new_table->malloc_size = 0;
        new_table->malloc_flags = 0;
        new_table->is_used = 0;
        main_table->next = new_table;
        return new_table;
    }
    else
    {
        // fprintf(stderr, "debug: given malloc table doesn't have next as null, can't create next here. checking next table's next.\n");
        return _lk_get_empty_malloc_table(main_table->next);
    }
}

/**
 * Returns a new empty entry for the data structure.
 * For getting an empty entry in the FREEs Table
 * 
 * Given an existing, filled table.
 * Should basically always be given first table.
 */
FREES_TABLE *_lk_get_empty_free_table(FREES_TABLE *main_table)
{
    if (main_frees_table == NULL)
    {
        // fprintf(stderr, "debug: main free table was null, creating first time table\n");
        main_frees_table = malloc(sizeof(FREES_TABLE));
        main_frees_table->next = NULL;
        main_frees_table->free_ptr = NULL;
        main_frees_table->approx_free_ptr = NULL;
        main_frees_table->orig_ptr = NULL;
        main_frees_table->free_size = 0;
        main_frees_table->free_flags = 0;
        main_frees_table->is_used = 0;
        main_frees_table->total_frees = 0; // originally 0
        return main_frees_table;
    }

    if (main_table->next == NULL)
    {
        // fprintf(stderr, "debug: creating a next free table and returning it\n");
        FREES_TABLE *new_table = malloc(sizeof(FREES_TABLE));
        new_table->next = NULL;
        new_table->free_ptr = NULL;
        new_table->approx_free_ptr = NULL;
        new_table->orig_ptr = NULL;
        new_table->free_size = 0;
        new_table->free_flags = 0;
        new_table->is_used = 0;
        main_table->next = new_table;
        return new_table;
    }
    else
    {
        // fprintf(stderr, "debug: given free table doesn't have next as null, can't create next here. checking next table's next.\n");
        return _lk_get_empty_free_table(main_table->next);
    }
}

/**
 * Attempt to find a FREEs table with the given ptr
 */
void *_lk_find_free_table_with_ptr(void **ptr)
{
    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        if (table->free_ptr == *ptr)
        {
            return table;
        }

        table = table->next;
    }

    return NULL;
}

/**
 * Find a record in the FREEs table that has its 'free_ptr' value equal to 'needle'
 * And where the 'after_this_time' value is a value smaller (before in time) of the 'microseconds' field of a FREE.
 * In short - finds a free that happened AFTER a malloc.
 */
void *_lk_find_free_table_with_ptr_after_time(void *needle, unsigned long after_this_time)
{
    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        if (table->free_ptr == needle)
        {
            // fprintf(stderr, "debug: found a free tble that matches the given needle (malloc) ptr. %lu\n", table->microseconds);
            if (table->microseconds >= after_this_time)
            {
                // fprintf(stderr, "debug: found the needle ptr in a table.\n");
                return table;
            }
        }

        table = table->next;
    }

    // fprintf(stderr, "debug: unable to find the needle ptr in the free's tables");
    return NULL;
}

/**
 * Similar to above - except looks in the mallocs table and for a malloc that happened before a free.
 */
void *_lk_find_malloc_table_with_ptr_before_time(void *needle, unsigned long before_this_time)
{
    MALLOCS_TABLE *table = main_mallocs_table;
    while (table != NULL)
    {
        if (table->malloc_ptr == needle)
        {
            if (table->microseconds <= before_this_time)
            {
                return table;
            }
        }

        table = table->next;
    }

    return NULL;
}

void _lk_print_malloc_table(FILE *out, MALLOCS_TABLE *table)
{
    fprintf(out, "0,%s,%s,%d,%lu,\"%p\",%d,%lu,\"%p\"\n", table->file, table->func, table->line, table->microseconds, table->orig_ptr, 0, table->malloc_size, table->malloc_ptr);

    // fprintf(out, "\tlkmalloc() called with pointer '%p' in %s:%s line: %d\n", table->malloc_ptr, table->file, table->func, table->line);
    // fprintf(out, "\t with timestamp %d (seconds) or %lu (microseconds)\n", table->timestamp, table->microseconds);
    return;
}
/**
 * Print out all the info of a FREE Table record
 */
void _lk_print_free_table(FILE *out, FREES_TABLE *table)
{
    void *ptr = table->free_ptr;
    if (table->approx_free_ptr != NULL)
    {
        ptr = table->approx_free_ptr;
    }

    fprintf(out, "1,%s,%s,%d,%lu,\"%p\",%d,0x%x,%s\n", table->file, table->func, table->line, table->microseconds, table->orig_ptr, 0, table->free_flags, "");
    ptr = ptr;
    // fprintf(out, "\tlkfree() called with pointer '%p' in %s:%s line: %d\n", ptr, table->file, table->func, table->line);
    // fprintf(out, "\t with timestamp %d (seconds) or %lu (microseconds)\n", table->timestamp, table->microseconds);
    return;
}

/**
 * Print out info on all the malloc's that have yet to of been free'd.
 * (where they don't have a corresponding free record.)
 */
void _lk_print_malloc_tables_with_no_frees(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_SERIOUS':\n");

    if (main_mallocs_table == NULL)
    {
        return;
    }

    MALLOCS_TABLE *table = main_mallocs_table;
    while (table != NULL)
    {
        // fprintf(stderr, "At microseconds %lu\n", m_table->microseconds);
        if (_lk_find_free_table_with_ptr_after_time(table->malloc_ptr, table->microseconds) == NULL)
        {
            _lk_print_malloc_table(out, table);
            // fprintf(out, "LKR_SERIOUS: %p, was never freed. Bytes lost: %llu. In %s:%s line:%d.\n", m_table->malloc_ptr, m_table->malloc_size, m_table->file, m_table->func, m_table->line);
        }

        table = table->next;
    }

    return;
}

/**
 * Print out info on all the malloc's that have a corresponding free.
 */
void _lk_print_malloc_free_matches(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_MATCH':\n");

    if (main_mallocs_table == NULL || main_frees_table == NULL)
    {
        return;
    }

    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        if (_lk_find_malloc_table_with_ptr_before_time(table->free_ptr, table->microseconds) != NULL)
        {
            _lk_print_free_table(out, table);
        }

        table = table->next;
    }

    return;
}

/**
 * Find a malloc that has it's ptr near the specified ptr... 
 * Whereas the ptr is within the bounds of the size+malloc_ptr.
 */
MALLOCS_TABLE *_lk_find_malloc_table_with_ptr_near(void **ptr, uint16_t flags)
{
    if (main_mallocs_table == NULL)
    {
        if (flags & LKF_UNKNOWN)
        {
            fprintf(stderr, "LKF_UNKNOWN: Warning - given pointer '%p' was never allocated.\n", *ptr);
        }

        if (flags & LKF_ERROR)
        {
            fprintf(stderr, "LKF_ERROR: exiting program.\n");
            exit(-LKF_ERROR);
        }

        return NULL;
    }

    MALLOCS_TABLE *table = main_mallocs_table;
    while (table != NULL)
    {
        if (*ptr == table->malloc_ptr)
        {
            return table;
        }

        if (*ptr >= table->malloc_ptr && *ptr <= table->malloc_ptr + table->malloc_size)
        {
            if (flags & LKF_WARN)
            {
                fprintf(stderr, "LKF_WARN: Warning - freeing pointer: '%p' - based on approximate given pointer: '%p'.\n", table->malloc_ptr, *ptr);
            }

            if (flags & LKF_ERROR)
            {
                fprintf(stderr, "LKF_ERROR: exiting program.\n");
                exit(-LKF_ERROR);
            }

            return table;
        }

        table = table->next;
    }

    if (flags & LKF_UNKNOWN)
    {
        fprintf(stderr, "LKF_UNKNOWN: Warning - given pointer '%p' was never allocated.\n", *ptr);
    }

    if (flags & LKF_ERROR)
    {
        fprintf(stderr, "LKF_ERROR: exiting program.\n");
        exit(-LKF_ERROR);
    }

    return NULL;
}

/**
 * Print out all the free's that weren't free'd via an exact address.
 * Iterates through the Free tables and prints out info on any that has it's 'approx_free_ptr' to anything that's not NULL.
 * LKR_BAD_FREE
 */
void _lk_print_bad_frees(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_BAD_FREE':\n");

    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        if (table->approx_free_ptr != NULL)
        {
            _lk_print_free_table(out, table);
        }

        table = table->next;
    }

    return;
}

/**
 * Print out all the free's
 */
void _lk_print_all_frees(FILE *out)
{
    fprintf(stderr, "Generating report for 'ALL FREES':\n");

    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        _lk_print_free_table(out, table);

        table = table->next;
    }

    return;
}
/**
 * Print out all the free's that were called on values that were never alloc'd
 * (orphan free's)
 */
void _lk_print_orphan_frees(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_ORPHAN_FREE':\n");

    if (main_frees_table == NULL)
    {
        return;
    }

    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        if (_lk_find_malloc_table_with_ptr_before_time(table->free_ptr, table->microseconds) == NULL)
        {
            _lk_print_free_table(out, table);
        }

        table = table->next;
    }

    return;
}

/**
 * Print out instances where a free was called twice on the same ptr (double free)
 */
void _lk_print_double_frees(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_DOUBLE_FREE':\n");

    if (main_frees_table == NULL)
    {
        return;
    }

    FREES_TABLE *table = main_frees_table;
    while (table != NULL)
    {
        // iterate thru the current table's next's until we reach end or find a table with ->free_ptr == table->ptr
        FREES_TABLE *table2 = table->next;
        while (table2 != NULL)
        {
            if (table2->free_ptr == table->free_ptr)
            {
                _lk_print_free_table(out, table);
                _lk_print_free_table(out, table2);
            }

            table2 = table2->next;
        }

        table = table->next;
    }

    return;
}

/**
 * Print out matching malloc and free pairs
 * where the free didn't use the exact address, but an approximate one.
 */
void _lk_print_approx_matches(FILE *out)
{
    fprintf(stderr, "Generating report for 'LKR_APPROX':\n");

    if (main_mallocs_table == NULL || main_frees_table == NULL)
    {
        return;
    }

    MALLOCS_TABLE *table = main_mallocs_table;
    while (table != NULL)
    {
        FREES_TABLE *table2 = main_frees_table;
        while (table2 != NULL)
        {
            if (table2->free_ptr == table->malloc_ptr)
            {
                if (table2->approx_free_ptr != NULL)
                {
                    _lk_print_malloc_table(out, table);
                    _lk_print_free_table(out, table2);
                    break;
                }
            }

            table2 = table2->next;
        }

        table = table->next;
    }

    return;
}

/**
 * Write the CSV headers out
 */
void write_csv_header(FILE *out)
{
    if (wrote_csv_header == 0)
    {
        wrote_csv_header = 1;
	setbuf(out, NULL);
	fprintf(out, "record_type,filename,fxname,line_num,timestamp,ptr_passed,retval,size_or_flags,alloc_addr_returned\n");
    }
}
