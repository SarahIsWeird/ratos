#ifndef VIRT_H
#define VIRT_H

#include <stddef.h>
#include <stdint.h>

#include <limine.h>

#define VIRT_PAGE_SIZE 4096

#define VIRT_ROUND_DOWN(addr) ((addr) & ~0xfff)
#define VIRT_ROUND_UP(addr)   (VIRT_ROUND_DOWN(addr) + !!((addr) & 0xfff))

#define VF_PRESENT         (1ll    <<  0)
#define VF_WRITABLE        (1ll    <<  1)
#define VF_USER            (1ll    <<  2)
#define VF_WRITE_THROUGH   (1ll    <<  3)
#define VF_CACHE_DISABLE   (1ll    <<  4)
#define VF_ACCESSED        (1ll    <<  5)
#define VF_PTE_DIRTY       (1ll    <<  6)
#define VF_PTE_PAT         (1ll    <<  7)
#define VF_PTE_GLOBAL      (1ll    <<  8)
#define VF_AVAIL1_MASK     (0xfll  <<  8)
#define VF_PTE_AVAIL2_MASK (0x7fll << 52)
#define VF_PTE_PK          (0xfll  << 59)
#define VF_EXECUTE_DISABLE (1ll    << 63)

typedef struct virt_ctx_t virt_ctx_t;
typedef uint64_t virt_flags_t;

uint64_t phys_to_virt(uint64_t phys);

void virt_init(struct limine_memmap_response *memmap_response, uint64_t hhdm_offset);

virt_ctx_t *virt_new(void);

void virt_use(virt_ctx_t *ctx);

uint64_t virt_alloc(virt_ctx_t *ctx, size_t length, virt_flags_t flags);
uint64_t virt_user_alloc(virt_ctx_t *ctx, size_t length, virt_flags_t flags);
void virt_free(virt_ctx_t *ctx, uint64_t addr);

void virt_map(virt_ctx_t *ctx, uint64_t phys, uint64_t virt, size_t length, virt_flags_t flags);

#endif
