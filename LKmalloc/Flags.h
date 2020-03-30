#ifndef FLAGS_H
#define FLAGS_H

/**
 * lkmalloc flags
 * 
 * LKM_REG - allocate memory without any of the special protections below.
 * LKM_INIT - initialize the memory being allocated to 0s.
 * LKM_OVER - allocate 8 more bytes of memory after the requested size, and write the pattern 0x5a in those upper bytes.
 * LKM_UNDER - allocate 8 more bytes of memory before the requested size, and write the pattern 0x6b in those lower bytes.
 */
#define LKM_REG 0x0
#define LKM_INIT 0x1
#define LKM_OVER 0x2
#define LKM_UNDER 0x4

/**
 * lkfree flags
 * 
 * LKF_REG 0x0: free only if the ptr passed was exactly as was allocated.
 * LKF_APPROX 0x1: free an allocation even if what is passed is in the middle of a valid allocation (normally free doesn't allow that).
 * LKF_WARN 0x2: print a warning if you free a ptr as per LKF_APPROX.
 * LKF_UNKNOWN 0x4: print a warning if asked to free a ptr that has never been allocated.
 * LKF_ERROR 0x8: exit the program if any condition matches LKF_WARN or LKF_UNKNOWN.
 */
#define LKF_REG 0x0
#define LKF_APPROX 0x1
#define LKF_WARN 0x2
#define LKF_UNKNOWN 0x4
#define LKF_ERROR 0x8

/**
 * lkreport flags
 * 
 * LKR_NONE 0x0: do not produce a report
 * LKR_SERIOUS 0x1: print memory leaks (e.g., mallocs w/o a corresponding free of the same addr)
 * LKR_MATCH 0x2: print perfectly matching alloc/free pairs
 * LKR_BAD_FREE 0x4: print bad ‘free's (ones where the passed addr is in middle of alloc addr)
 * LKR_ORPHAN_FREE 0x8: print orphan ‘free's (ones that had never been allocated)
 * LKR_DOUBLE_FREE 0x10: print double free’d pointers
 * LKR_APPROX 0x20: print matching alloc/free pairs that were freed due to LKF_APPROX above.
 */
#define LKR_NONE 0x0
#define LKR_SERIOUS 0x1
#define LKR_MATCH 0x2
#define LKR_BAD_FREE 0x4
#define LKR_ORPHAN_FREE 0x8
#define LKR_DOUBLE_FREE 0x10
#define LKR_APPROX 0x20

#endif
