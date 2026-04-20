#include "gdt.h"

#define GDT_ENTRIES 5

gdt_entry_t gdt[GDT_ENTRIES];
gdtr_t gdtr;

extern void gdt_reload_segments(void);

static void s_add_gdt_entry(int index, uint64_t base, uint64_t limit, uint64_t access, uint64_t flags) {
    gdt[index].base_low = base & 0xffffff;
    gdt[index].base_high = (base >> 24) & 0xff;
    gdt[index].limit_low = limit & 0xffff;
    gdt[index].limit_high = (limit >> 16) & 0xf;
    gdt[index].access = access & 0xff;
    gdt[index].flags = flags & 0xf;
}

static void s_load_gdt(void) {
    __asm__ ("lgdt %0" :: "m" (gdtr));
    gdt_reload_segments();
}

void gdt_init(void) {
    s_add_gdt_entry(0, 0, 0, 0, 0);
    s_add_gdt_entry(1, 0, 0xfffff,
        GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_READABLE | GDT_ACCESS_ACCESSED,
        GDT_FLAGS_PAGE_GRANULARITY | GDT_FLAGS_LONG_MODE_CODE);
    s_add_gdt_entry(2, 0, 0xfffff,
        GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_WRITABLE | GDT_ACCESS_ACCESSED,
        GDT_FLAGS_PAGE_GRANULARITY | GDT_FLAGS_PROTECTED_MODE);
    s_add_gdt_entry(3, 0, 0xfffff,
        GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_READABLE | GDT_ACCESS_ACCESSED,
        GDT_FLAGS_PAGE_GRANULARITY | GDT_FLAGS_LONG_MODE_CODE);
    s_add_gdt_entry(4, 0, 0xfffff,
        GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_WRITABLE | GDT_ACCESS_ACCESSED,
        GDT_FLAGS_PAGE_GRANULARITY | GDT_FLAGS_PROTECTED_MODE);
    
    gdtr.size = sizeof(uint64_t) * GDT_ENTRIES - 1;
    gdtr.offset = (uint64_t) gdt;

    s_load_gdt();
}
