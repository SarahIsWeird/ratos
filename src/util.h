#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define LIMINE_REQUEST __attribute__((used, section(".limine_requests"))) static volatile

static inline void hcf(void) {
    while (1) {
        __asm__ volatile("cli; hlt");
    }

    __builtin_unreachable();
}

static inline uint64_t get_bitmask64(unsigned int start, unsigned int end) {
    uint64_t mask = 0;

    for (unsigned int i = start; i <= end; i++) {
        mask |= 1ll << i;
    }

    return mask;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" :: "a" (value), "Nd" (port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" :: "a" (value), "Nd" (port));
}

static inline void outd(uint16_t port, uint32_t value) {
    __asm__ volatile ("outd %0, %1" :: "a" (value), "Nd" (port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile ("inw %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline uint32_t ind(uint16_t port) {
    uint32_t result;
    __asm__ volatile ("ind %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

#endif /* UTIL_H */
