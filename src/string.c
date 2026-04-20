#include "string.h"

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

size_t strlen(const char *str) {
    size_t length = 0;
    while (*(str++)) {
        length++;
    }

    return length;
}
