#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void memset(void *dst, int value, size_t count);
void memcpy(void *dst, const void *src, size_t count);
void *memchr(void *ptr, int ch, size_t count);
int memcmp(const void *a, const void *b, size_t n);

size_t strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(char *str, int ch);
void strcpy(char *dst, const char *src);
void strncpy(char *dst, const char *src, size_t length);

#endif /* STRING_H */
