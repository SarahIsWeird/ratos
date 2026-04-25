#include "string.h"

#include <stdbool.h>
#include <stdint.h>

void memset(void *dst, int value, const size_t count) {
    uint8_t *dst_ = dst;
    uint8_t value_ = (uint8_t) value;

    for (size_t i = 0; i < count; i++) {
        dst_[i] = value_;
    }
}

void memcpy(void *dst, const void *src, const size_t count) {
    uint8_t *dst_ = dst;
    const uint8_t *src_ = src;

    for (size_t i = 0; i < count; i++) {
        dst_[i] = src_[i];
    }
}

void *memchr(void *ptr, int ch, size_t count) {
    unsigned char *byte_ptr = (unsigned char *) ptr;

    for (size_t i = 0; i < count; i++) {
        if (byte_ptr[i] == (unsigned char) ch) {
            return ptr + i;
        }
    }

    return NULL;
}

size_t strlen(const char *str) {
    size_t length = 0;
    while (*(str++)) {
        length++;
    }

    return length;
}

int strcmp(const char *a, const char *b) {
    int diff = 0;

    do {
        diff = (unsigned char) *a - (unsigned char) *b;
        if (diff != 0) return diff;

        a++;
        b++;
    } while (*a != 0 && *b != 0);

    return diff;
}

int strncmp(const char *a, const char *b, size_t n) {
    int diff = 0;
    size_t i = 0;

    if (n == 0) return 0;

    do {
        diff = (unsigned char) *a - (unsigned char) *b;
        if (diff != 0) return diff;

        a++;
        b++;
        i++;
    } while (*a != 0 && *b != 0 && i < n);

    return diff;
}

char *strchr(char *str, int ch) {
    while (*str != 0) {
        if (*str == (char) ch) return str;
        str++;
    }

    return NULL;
}

void strcpy(char *dst, const char *src) {
    while (*src) {
        *(dst++) = *(src++);
    }

    *dst = 0;
}

void strncpy(char *dst, const char *src, size_t length) {
    size_t i = 0;
    for (; i < length && src[i] != 0; i++) {
        dst[i] = src[i];
    }

    dst[i] = 0;
}
