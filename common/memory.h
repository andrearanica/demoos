#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>

void memzero(unsigned long src, unsigned long n);
int memcmp(const void *src1, const void *src2, size_t n);
void memset(void *dest, int c, size_t count);
void memcpy(void *dest, void *src, size_t count);

#endif
