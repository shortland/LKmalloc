#ifndef LKMALLOC_H
#define LKMALLOC_H

#include <stdint.h>
#include "Flags.h"

#define lkmalloc(size, ptr, flags) _lkmalloc_internal((size), (ptr), (flags), __FILE__, __func__, __LINE__)
int _lkmalloc_internal(uint64_t size, void **ptr, uint16_t flags, const char *file, const char *func, int line);

#define lkfree(ptr, flags) _lkfree_internal((ptr), (flags), __FILE__, __func__, __LINE__, NULL, (flags))
int _lkfree_internal(void **ptr, uint16_t flags, const char *file, const char *func, int line, void **original, uint16_t rflags);

int lkreport(int fd, uint16_t flags);

#endif