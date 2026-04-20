#include "virt.h"

#include <stdbool.h>

#include "cpuid.h"
#include "drv/term.h"
#include "phys.h"
#include "util.h"

#define LIN_MASK   0xfff
#define PT_OFFSET 12
#define PD_OFFSET 21
#define PDPT_OFFSET 30
#define PM4_OFFSET 39
#define PM5_OFFSET 48
#define INDEX_MASK 0x1ff

#define PHYS_MASK ((uint64_t) 0x7ffffffffffffc00)

struct virt_ctx_t {
    uint64_t phys;
    uint64_t virt;
};

static uint64_t s_hhdm_offset = 0;
static unsigned int s_max_phy_addr = 0;
static uint64_t s_phy_mask = 0;

static uint64_t boobies = 0xcafebabedeadbeef;

static uint64_t s_sanitize_page_structure_flags(virt_flags_t flags, bool is_user_struct) {
    // TODO: take into account if it's in the user address space and add the user flag
    // if the PDE (or PDPTE or PML4E or PML5E) doesn't have the user flag set it no worky then
    return flags;
}

static uint64_t *s_get_struct_entry(uint64_t *pstruct, uint64_t index, bool create_structure, virt_flags_t flags) {
    uint64_t entry = pstruct[index];
    if (entry != 0) return &pstruct[index];
    if (!create_structure) return NULL;

    uint64_t phy_addr = phys_alloc();
    if (phy_addr == NULL) {
        kerror("Out of memory while trying to allocate space for a paging structure!\n");
        hcf();
    }

    if (phy_addr != (s_phy_mask & phy_addr)) {
        kerror("Tried to allocate physical memory for a paging structure at %0p, but it doesn't fit into the physical address space! (mask %0p)\n", phy_addr, s_phy_mask);
        hcf();
    }

    pstruct[index] = phy_addr | flags;
    return (uint64_t *) phys_to_virt(phy_addr);
}

static uint64_t *s_entry_to_pointer(uint64_t *entry_ptr) {
    return (uint64_t *) phys_to_virt(*entry_ptr & PHYS_MASK);
}

static uint64_t *s_get_pte(virt_ctx_t *ctx, uint64_t addr, bool create_structures, virt_flags_t flags) {
    uint64_t *top_level;
    if (ctx == NULL) {
        uint64_t cr3;
        __asm__ ("mov %%cr3, %0" : "=r" (cr3));
        top_level = (uint64_t *) phys_to_virt(cr3);
    } else {
        top_level = (uint64_t *) ctx->virt;
    }

    uint64_t *pml4 = top_level;
    uint64_t *pml4e = s_get_struct_entry(pml4, (addr >> PM4_OFFSET) & INDEX_MASK, create_structures, flags);
    kdebug("pml4: %0p, pml4e: %0p, index: %llu\n", pml4, pml4e, (addr >> PM4_OFFSET) & INDEX_MASK);
    if (pml4e == NULL) return NULL;

    uint64_t *pdpt = s_entry_to_pointer(pml4e);
    uint64_t *pdpte = s_get_struct_entry(pdpt, (addr >> PDPT_OFFSET) & INDEX_MASK, create_structures, flags);
    kdebug("pdpt: %0p, pdpte: %0p, index: %llu\n", pdpt, pdpte, (addr >> PDPT_OFFSET) & INDEX_MASK);
    if (pdpte == NULL) return NULL;

    uint64_t *pd = s_entry_to_pointer(pdpte);
    uint64_t *pde = s_get_struct_entry(pd, (addr >> PD_OFFSET) & INDEX_MASK, create_structures, flags);
    kdebug("pd: %0p, pde: %0p, index: %llu\n", pd, pde, (addr >> PD_OFFSET) & INDEX_MASK);
    if (pde == NULL) return NULL;

    uint64_t *pt = s_entry_to_pointer(pde);
    uint64_t *pte = s_get_struct_entry(pt, (addr >> PT_OFFSET) & INDEX_MASK, create_structures, flags);
    kdebug("pt: %0p, pte: %0p, index: %llu\n", pt, pte, (addr >> PT_OFFSET) & INDEX_MASK);
    return pte;
}

static uint64_t s_find_virt_region(virt_ctx_t *ctx, uint64_t start, uint64_t end, uint64_t length) {
    length = VIRT_ROUND_UP(length);

    uint64_t start = start;
    while (start < end) {
while_cont:
        for (size_t i = 0; i < length; i += VIRT_PAGE_SIZE) {
            if (s_get_pte(ctx, start + i, false, 0)) {
                start += i + VIRT_PAGE_SIZE;
                goto while_cont;
            }
        }

        return start;
    }

    return 0;
}

uint64_t phys_to_virt(uint64_t phys) {
    return (phys + s_hhdm_offset);
}

void virt_init(struct limine_memmap_response *memmap_response, uint64_t hhdm_offset) {
    s_hhdm_offset = hhdm_offset;

    uint32_t asr = cpuid_eax(CPUID_ADDRESS_SPACE_INFO, 0);
    s_max_phy_addr = (asr >> CPUID_ASI_EAX_PHYS_ADDR_SIZE_OFFSET) & CPUID_ASI_EAX_PHYS_ADDR_SIZE_MASK;
    uint32_t linear_addr_size = (asr >> CPUID_ASI_EAX_LIN_ADDR_SIZE_OFFSET) & CPUID_ASI_EAX_LIN_ADDR_SIZE_MASK;
    kdebug("Physical address size: %u bits, linear address size: %u bits\n", s_max_phy_addr, linear_addr_size);

    s_phy_mask = get_bitmask64(12, s_max_phy_addr - 1);

    uint64_t *curr_virt = &boobies;
    kinfo("boobies: %0p\n", curr_virt);
    uint64_t *pte_entry = s_get_pte(NULL, (uint64_t) curr_virt, false, 0);
    kinfo("pte entry: %0p\n", pte_entry);
    uint64_t *phys = s_entry_to_pointer(pte_entry);
    kinfo("phys addr: %0p\n", phys);
    uint64_t offset = (uint64_t) curr_virt & INDEX_MASK;
    uint64_t virt = (uint64_t) phys + offset;
    kinfo("addr: %0p, value: %0p\n", virt, *(uint64_t *) virt);
}

virt_ctx_t *virt_new(void) {
    virt_ctx_t *ctx = (virt_ctx_t *) virt_alloc(NULL, sizeof(virt_ctx_t), 0);
    return ctx;
}

void virt_use(virt_ctx_t *ctx) {
    __asm__ volatile("mov %0, %%cr3" :: "r" (ctx->phys) : "memory");
}

uint64_t virt_alloc(virt_ctx_t *ctx, size_t length, virt_flags_t flags) {
    uint64_t start = s_find_virt_region(ctx, s_hhdm_offset, 0xfffffffffffff000, length);
    if (start == 0) return 0;

    for (uint64_t i = start; i < start + length; i += VIRT_PAGE_SIZE) {
        uint64_t phys = phys_alloc();
        if (phys == 0) return 0; // fixme: memleak when oom
        virt_map(ctx, phys, i, VIRT_PAGE_SIZE, flags | VF_PRESENT);
    }

    return start;
}

uint64_t virt_user_alloc(virt_ctx_t *ctx, size_t length, virt_flags_t flags) {

}

void virt_free(virt_ctx_t *ctx, uint64_t addr) {

}

void virt_map(virt_ctx_t *ctx, uint64_t phys, uint64_t virt, size_t length, virt_flags_t flags) {

}
