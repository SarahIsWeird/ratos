#ifndef VIRT_H
#define VIRT_H

#include <stddef.h>
#include <stdint.h>

#define VIRT_ENTRIES_PER_STRUCT 512
#define VIRT_PAGE_SIZE          4096
#define VIRT_PAGE_SIZE_BIG      (VIRT_PAGE_SIZE * VIRT_ENTRIES_PER_STRUCT)

#define VF_PT_PRESENT       0x001
#define VF_PT_WRITABLE      0x002
#define VF_PT_USER          0x004
#define VF_PT_WRITE_THROUGH 0x008
#define VF_PT_CACHE_DISABLE 0x010
#define VF_PT_ACCESSED      0x020
#define VF_PT_DIRTY         0x040
#define VF_PT_PAT           0x080
#define VF_PT_GLOBAL        0x100

#define VF_PD_PRESENT       0x001
#define VF_PD_WRITABLE      0x002
#define VF_PD_USER          0x004
#define VF_PD_WRITE_THROUGH 0x008
#define VF_PD_CACHE_DISABLE 0x010
#define VF_PD_ACCESSED      0x020
#define VF_PD_DIRTY         0x040
#define VF_PD_BIG_PAGE      0x080
#define VF_PD_GLOBAL        0x100

#define VIRT_PML4_INDEX_OFFSET 39
#define VIRT_PDPT_INDEX_OFFSET 30
#define VIRT_PD_INDEX_OFFSET   21
#define VIRT_PT_INDEX_OFFSET   12
#define VIRT_INDEX_MASK        0x1ff

typedef struct virt_ctx_t virt_ctx_t;

void virt_init(void);

virt_ctx_t *virt_ctx_new(void);
void virt_ctx_use(virt_ctx_t *ctx);

uint64_t virt_alloc(virt_ctx_t *ctx, size_t size);
uint64_t virt_user_alloc(virt_ctx_t *ctx, size_t size);
void virt_free(virt_ctx_t *ctx, uint64_t addr, size_t size);

void virt_map(virt_ctx_t *ctx, uint64_t phys_addr, uint64_t virt_addr, size_t size, uint64_t flags);
void virt_map_user(virt_ctx_t *ctx, uint64_t phys_addr, uint64_t virt_addr, size_t size, uint64_t flags);
void virt_unmap(virt_ctx_t *ctx, uint64_t addr, size_t size);

#endif /* VIRT_H */
