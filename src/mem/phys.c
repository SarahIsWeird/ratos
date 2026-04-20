#include "phys.h"

#include <limine.h>

#include "drv/term.h"
#include "string.h"
#include "util.h"


static size_t s_address_space_size = 0;
static size_t s_pmm_bitmap_size = 0;
static uint64_t *s_pmm_bitmap = NULL;

static inline size_t s_get_bitmap_index(uint64_t addr) {
    return addr / PHYS_CHUNK_SIZE / sizeof(uint64_t);
}

static inline int s_get_bitmap_shift(uint64_t addr) {
    return (addr / PHYS_CHUNK_SIZE) % sizeof(uint64_t);
}

static inline int s_get_bitmap_mask(uint64_t addr) {
    return 1 << s_get_bitmap_shift(addr);
}

static inline uint64_t s_phys_is_used(uint64_t addr) {
    return s_pmm_bitmap[s_get_bitmap_index(addr)] & s_get_bitmap_mask(addr);
}

void phys_init(struct limine_memmap_request memmap_request, uint64_t hhdm_offset) {
    if (memmap_request.response == NULL || memmap_request.response->entry_count == 0) {
        kerror("Didn't get a memory map from Limine!\n");
        hcf();
    }

    struct limine_memmap_response *memmap = memmap_request.response;
    size_t memmap_entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **memmap_entries = memmap_request.response->entries;

    s_address_space_size = PHYS_ROUND_UP(memmap_entries[memmap_entry_count - 1]->base + memmap_entries[memmap_entry_count - 1]->length);
    s_pmm_bitmap_size = s_address_space_size / PHYS_CHUNK_SIZE / 8; // in bytes
    for (size_t i = 0; i < memmap_entry_count; i++) {
        size_t entry_base = memmap_entries[i]->base;
        size_t entry_length = memmap_entries[i]->length;
        size_t entry_type = memmap_entries[i]->type;

        if (entry_type != LIMINE_MEMMAP_USABLE) continue;

        if (entry_base == 0) {
            // i *really* do not want to alloc it at NULL lol
            entry_base += PHYS_CHUNK_SIZE;
            entry_length--;
        }

        if (entry_length >= s_pmm_bitmap_size) {
            // this segment of the address space can hold the memory map, so we dump it in there
            s_pmm_bitmap = (uint64_t *) (entry_base + hhdm_offset);
        }
    }

    memset(s_pmm_bitmap, 0xff, s_pmm_bitmap_size);

    if (s_pmm_bitmap == NULL) {
        kerror("Couldn't find a space to put the %llu bytes long pmm bitmap!\n", s_pmm_bitmap_size);
        hcf();
    }

    kdebug("Entries: %llu\n", memmap->entry_count);
    for (size_t i = 0; i < memmap->entry_count; i++) {
        kdebug("Base: %0p, length: %0p, type: %lld\n", memmap->entries[i]->base, memmap->entries[i]->length, memmap->entries[i]->type);

        if (memmap->entries[i]->type != LIMINE_MEMMAP_USABLE) continue;
        phys_mark_region_unused(memmap->entries[i]->base, memmap->entries[i]->length);
    }


    phys_mark_region_used((uint64_t) s_pmm_bitmap - hhdm_offset, s_pmm_bitmap_size);
    phys_mark_used(0);
}

uint64_t phys_alloc(void) {
    return phys_alloc_contiguous(PHYS_CHUNK_SIZE);
}

uint64_t phys_alloc_contiguous(size_t length) {
    length = PHYS_ROUND_UP(length);

    uint64_t start = 0;
    while (start < s_address_space_size) {
while_cont:
        for (size_t i = 0; i < length; i += PHYS_CHUNK_SIZE) {
            if (s_phys_is_used(start + i)) {
                start += i + PHYS_CHUNK_SIZE;
                goto while_cont;
            }
        }

        phys_mark_region_used(start, length);
        return start;
    }

    return 0;
}

void phys_free(uint64_t addr) {
    phys_mark_unused(addr);
}

void phys_mark_used(uint64_t addr) {
    s_pmm_bitmap[s_get_bitmap_index(addr)] |= s_get_bitmap_mask(addr);
}

void phys_mark_region_used(uint64_t base, size_t length) {
    for (size_t i = 0; i < PHYS_ROUND_UP(length); i += PHYS_CHUNK_SIZE) {
        phys_mark_used(base + i);
    }
}

void phys_mark_unused(uint64_t addr) {
    s_pmm_bitmap[s_get_bitmap_index(addr)] &= ~s_get_bitmap_mask(addr);
}

void phys_mark_region_unused(uint64_t base, size_t length) {
    for (size_t i = 0; i < PHYS_ROUND_DOWN(length); i += PHYS_CHUNK_SIZE) {
        phys_mark_unused(base + i);
    }
}

uint64_t phys_get_address_space_size(void) {
    return s_address_space_size;
}
