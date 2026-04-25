#ifndef PHYS_H
#define PHYS_H

#include <stddef.h>
#include <stdint.h>

#define PHYS_PAGE_SIZE 4096

void phys_init(void);

void phys_mark_used(uint64_t addr);
void phys_mark_unused(uint64_t addr);

void phys_mark_region_used(uint64_t addr, size_t length);
void phys_mark_region_unused(uint64_t addr, size_t length);

uint64_t phys_alloc(void);
uint64_t phys_alloc_region(size_t length);
void phys_free(uint64_t addr);
void phys_free_region(uint64_t addr, size_t length);

uint64_t phys_get_address_space_size(void);

#endif /* PHYS_H */
