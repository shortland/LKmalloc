#ifndef LKMALLOC_H
#define LKMALLOC_H

#include <stdint.h>
#include "Flags.h"
#include "LKmalloc.c"

#define lkmalloc(size, ptr, flags) __lkmalloc_internal((size), (ptr), (flags), __FILE__, __func__, __LINE__)

#define lkfree(ptr, flags) __lkfree_internal((ptr), (flags), __FILE__, __func__, __LINE__, NULL, (flags))

int lkreport(int fd, uint16_t flags);

#endif