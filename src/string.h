#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void memset(void *dst, int value, size_t count);
void memcpy(void *dst, const void *src, size_t count);

size_t strlen(const char *str);

#endif /* STRING_H */
