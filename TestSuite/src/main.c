#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "LKmalloc.h"

int main(int argc, char *argv[])
{
    /**
     * Try and write the output to a CSV file
     */
    FILE *csv = fopen("test_out_file.csv", "w");
    if (csv == NULL)
    {
        fprintf(stderr, "error: there was an error trying to open the file for writing on line %d.\n", __LINE__);
    }

    /**
     * Test 1:
     * 4 mallocs, and 2 frees - results in 2 memory leaks & 2 matching mallocs.
     */
    fprintf(stdout, "\nTest 1: 2 memleaks and 2 matching malloc-free's on line %d.\n", __LINE__);

    char *tmp1 = NULL;
    if (lkmalloc(16, (void **)(&tmp1), LKM_REG) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkmalloc on line %d.\n", __LINE__);
    }

    char *tmp2;
    if (lkmalloc(32, (void **)(&tmp2), LKM_REG) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkmalloc on line %d.\n", __LINE__);
    }

    char *tmp3;
    if (lkmalloc(48, (void **)(&tmp3), LKM_REG) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkmalloc on line %d.\n", __LINE__);
    }

    // free tmp2
    if (lkfree((void **)(&tmp2), LKF_REG) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error on line %d.\n", __LINE__);
    }

    char *tmp4;
    if (lkmalloc(64, (void **)(&tmp4), LKM_REG) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkmalloc on line %d.\n", __LINE__);
    }

    // free tmp4
    if (lkfree((void **)(&tmp4), LKF_REG) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error.%d\n", errno);
        errno = errno * -1;
        perror("error was");
    }

    // report that should show the 2 memleaks and 2 matching mallocs
    if (lkreport(fileno(csv), LKR_SERIOUS | LKR_MATCH) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkreport on line %d.\n", __LINE__);
    }

    /**
     * Cleanup Test 1
     */
    if (lkfree((void **)(&tmp1), LKF_REG) < 0 || lkfree((void **)(&tmp3), LKF_REG) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error.%d\n", errno);
        errno = errno * -1;
        perror("error was");
    }

    /**
     * Test 2:
     * 1 malloc with setting the space to 0's initially
     */
    fprintf(stdout, "\nTest 2: Attempting to malloc with it's space initialized to 0's.\n");

    char *cleared_space = NULL;
    if (lkmalloc(8, (void **)(&cleared_space), LKM_INIT) < 0)
    {
        fprintf(stderr, "error: lkmalloc() returned with an error on line %d.\n", __LINE__);
    }

    fprintf(stdout, "The space is: '%d%d%d%d%d%d%d%d' (showing as decimal values)\n", cleared_space[0], cleared_space[1], cleared_space[2], cleared_space[3], cleared_space[4], cleared_space[5], cleared_space[6], cleared_space[7]);

    /**
     * Cleanup Test 2
     */
    if (lkfree((void **)(&cleared_space), LKF_REG) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error on line %d.\n", __LINE__);
    }

    /**
     * Test 3:
     * Free a non-valid address (should be error)
     */
    fprintf(stdout, "\nTest 3: Free an address that isn't valid (wasn't previously malloc'd).\n");

    char *invalid_free = "hello world";
    if (lkfree((void **)(&invalid_free), LKF_REG | LKF_UNKNOWN) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error on line %d.\n", __LINE__);
    }

    /**
     * Test 4:
     * Pass address that's not the exact address of a malloc (it's within the address space of the malloc)
     */
    fprintf(stdout, "\nTest 4: Free the middle of a malloc'd address.\n");

    char *middle_free_original = "abc";
    if (lkmalloc(48, (void **)(&middle_free_original), LKM_INIT) < 0)
    {
        fprintf(stderr, "error: lkmalloc() returned with an error on line %d.\n", __LINE__);
    }

    void **middle_free_wrong = (void **)(&middle_free_original);
    (*middle_free_wrong)++; // purposely mess up the address slightly

    fprintf(stdout, "Using: the ptr %p for lkfree() instead of (correct) pointer: %p.\n", *middle_free_wrong, *middle_free_wrong - 1);

    if (lkfree(middle_free_wrong, LKF_APPROX | LKF_WARN) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error on line %d.\n", __LINE__);
    }

    if (lkfree(middle_free_wrong, LKF_APPROX | LKF_WARN) < 0)
    {
        fprintf(stderr, "error: lkfree() returned with an error on line %d.\n", __LINE__);
    }

    fprintf(stdout, "\nMisc. Reports.\n");
    if (lkreport(fileno(csv), LKR_BAD_FREE) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkreport on line %d.\n", __LINE__);
    }

    if (lkreport(fileno(csv), LKR_ORPHAN_FREE) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkreport on line %d.\n", __LINE__);
    }

    if (lkreport(fileno(csv), LKR_DOUBLE_FREE) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkreport on line %d.\n", __LINE__);
    }

    if (lkreport(fileno(csv), LKR_APPROX) < 0)
    {
        fprintf(stderr, "error: there was an error calling lkreport on line %d.\n", __LINE__);
    }

    fprintf(stdout, "done.\n");

    return 0;
}
