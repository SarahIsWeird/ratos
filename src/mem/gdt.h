#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_ACCESS_PRESENT      0x80
#define GDT_ACCESS_DPL_RING0    0x00
#define GDT_ACCESS_DPL_RING3    0x60
#define GDT_ACCESS_CODE_SEGMENT 0x18
#define GDT_ACCESS_DATA_SEGMENT 0x10
#define GDT_ACCESS_CONFORMING   0x04
#define GDT_ACCESS_READABLE     0x02
#define GDT_ACCESS_WRITABLE     0x02
#define GDT_ACCESS_ACCESSED     0x01

#define GDT_FLAGS_PAGE_GRANULARITY 0x8
#define GDT_FLAGS_PROTECTED_MODE   0x4
#define GDT_FLAGS_LONG_MODE_CODE   0x2

typedef struct __attribute__((__packed__)) gdt_entry_t {
    uint64_t limit_low  : 16;
    uint64_t base_low   : 24;
    uint64_t access     :  8;
    uint64_t limit_high :  4;
    uint64_t flags      :  4;
    uint64_t base_high  :  8;
} gdt_entry_t;

typedef struct __attribute__((__packed__)) gdtr_t {
    uint16_t size;
    uint64_t offset;
} gdtr_t;

void gdt_init(void);

#endif /* GDT_H */
