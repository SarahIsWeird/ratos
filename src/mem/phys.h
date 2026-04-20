#ifndef PHYS_H
#define PHYS_H

#include <stddef.h>
#include <stdint.h>

#include <limine.h>

#define PHYS_CHUNK_SIZE 4096

#define PHYS_ROUND_DOWN(addr) ((addr) & ~0xfff)
#define PHYS_ROUND_UP(addr) (PHYS_ROUND_DOWN(addr) + !!((addr) & 0xfff))

void phys_init(struct limine_memmap_request memmap_request, uint64_t hhdm_offset);

uint64_t phys_alloc(void);
uint64_t phys_alloc_contiguous(size_t length);
void phys_free(uint64_t addr);

void phys_mark_used(uint64_t addr);
void phys_mark_region_used(uint64_t base, size_t length);
void phys_mark_unused(uint64_t addr);
void phys_mark_region_unused(uint64_t base, size_t length);

uint64_t phys_get_address_space_size(void);

#endif /* PHYS_H */
