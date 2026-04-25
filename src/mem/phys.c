#include "phys.h"

#include "drv/term.h"
#include "limine_data.h"
#include "string.h"
#include "util.h"

static uint64_t s_phys_hhdm_offset;
static uint64_t *s_phys_bitmap = NULL;
static size_t s_phys_bitmap_size;
static size_t s_phys_addr_space_size;

#define PHYS_BITMAP_INDEX(addr)   ((uint64_t) (addr) / PHYS_PAGE_SIZE / 64)
#define PHYS_BITMAP_BIT_POS(addr) (((uint64_t) (addr) / PHYS_PAGE_SIZE) % 64)
#define PHYS_BITMAP_MASK(addr)    (1ull << PHYS_BITMAP_BIT_POS(addr))

#define FULLY_USED (~0ull)

static const char *memmap_type_names[] = {
    "usable",
    "reserved",
    "ACPI reclaimable",
    "ACPI non-volatile storage",
    "bad memory",
    "bootloader reclaimable",
    "executable and modules",
    "framebuffer",
    "reserved mapped",
};

void phys_init(void) {
    s_phys_hhdm_offset = limine_data_hhdm_get_offset();

    struct limine_memmap_response *memmap_response = limine_data_get_memmap_response();
    size_t memmap_entry_count = memmap_response->entry_count;
    struct limine_memmap_entry **memmap_entries = memmap_response->entries;

    struct limine_memmap_entry *last_entry = memmap_entries[memmap_entry_count - 1];
    s_phys_addr_space_size = last_entry->base + last_entry->length;
    s_phys_bitmap_size = ceildivu64(s_phys_addr_space_size, PHYS_PAGE_SIZE * BITS_PER_BYTE);
    kinfo("addr space size: %0p, bitmap size: %0p\n", s_phys_addr_space_size, s_phys_bitmap_size);

    for (size_t i = 0; i < memmap_entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_entries[i];
        size_t base = entry->base;
        size_t length = entry->length;
        if (base == 0) {
            base += PHYS_PAGE_SIZE;
            length -= PHYS_PAGE_SIZE;
        }

        if ((entry->type == LIMINE_MEMMAP_USABLE) && (length > s_phys_bitmap_size)) {
            s_phys_bitmap = (uint64_t *) (base + s_phys_hhdm_offset);
            break;
        }
    }

    if (!s_phys_bitmap) {
        kerror("Failed to find a memory region big enough to hold the physical memory map!\n");
        hcf();
    }

    memset(s_phys_bitmap, 0xff, s_phys_bitmap_size);

    kinfo("Physical memory regions:\n");
    for (size_t i = 0; i < memmap_entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            phys_mark_region_unused(entry->base, entry->length);
        }

        kinfo("%0p - %0p (%s)\n", entry->base, entry->base + entry->length, memmap_type_names[entry->type]);
    }

    phys_mark_region_used((uint64_t) s_phys_bitmap - s_phys_hhdm_offset, s_phys_bitmap_size * sizeof(uint64_t));

    phys_mark_used(0);
}

void phys_mark_used(uint64_t addr) {
    s_phys_bitmap[PHYS_BITMAP_INDEX(addr)] |= PHYS_BITMAP_MASK(addr);
}

void phys_mark_unused(uint64_t addr) {
    s_phys_bitmap[PHYS_BITMAP_INDEX(addr)] &= ~PHYS_BITMAP_MASK(addr);
}

void phys_mark_region_used(uint64_t addr, size_t length) {
    for (size_t i = 0; i < length; i += PHYS_PAGE_SIZE) {
        phys_mark_used(addr + i);
    }
}

void phys_mark_region_unused(uint64_t addr, size_t length) {
    for (size_t i = 0; i < length; i += PHYS_PAGE_SIZE) {
        phys_mark_unused(addr + i);
    }
}

uint64_t phys_alloc(void) {
    for (size_t i = 0; i < s_phys_bitmap_size; i++) {
        uint64_t entry = s_phys_bitmap[i];
        if (entry == FULLY_USED) continue;

        for (size_t bit = 0; bit < 64; bit++) {
            uint64_t mask = 1ull << bit;
            if (entry & mask) continue;

            s_phys_bitmap[i] |= mask;
            return (i * 64ull + bit) * PHYS_PAGE_SIZE;
        }
    }

    return 0;
}

uint64_t phys_alloc_region(size_t length) {
    for (size_t addr = 0; addr < s_phys_addr_space_size;) {
        bool found = true;
        for (size_t i = 0; i < length; i += PHYS_PAGE_SIZE) {
            if (s_phys_bitmap[PHYS_BITMAP_INDEX(addr + i)] & PHYS_BITMAP_MASK(addr + i)) {
                found = false;
                addr += i + PHYS_PAGE_SIZE;
                break;
            }
        }

        if (found) {
            phys_mark_region_used(addr, length);
            return addr;
        }
    }

    return 0;
}

void phys_free(uint64_t addr) {
    s_phys_bitmap[PHYS_BITMAP_INDEX(addr)] &= ~PHYS_BITMAP_MASK(addr);
}

void phys_free_region(uint64_t addr, size_t length) {
    for (size_t i = 0; i < length; i += PHYS_PAGE_SIZE) {
        phys_free(addr + i);
    }
}

uint64_t phys_get_address_space_size(void) {
    return s_phys_addr_space_size;
}
