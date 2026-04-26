#include "virt.h"

#include "debug/stacktrace.h"
#include "drv/term.h"
#include "limine_data.h"
#include "mem/phys.h"
#include "string.h"
#include "util.h"

#define ENTRY_PHYS_ADDR_MASK 0x001ffffffffff000ull

struct virt_ctx_t {
    uint64_t phys;
    uint64_t *virt;
};

static uint64_t s_virt_hhdm_offset;
static uint64_t s_kernel_pdpt_virt;

extern uint64_t kernel_start;
extern uint64_t kernel_end;

static inline uint64_t s_phys_to_virt(uint64_t addr) {
    return addr + s_virt_hhdm_offset;
}

static inline uint64_t s_virt_to_phys(uint64_t addr) {
    return addr - s_virt_hhdm_offset;
}

static uint64_t *s_virt_get_top_level(virt_ctx_t *ctx) {
    if (ctx != NULL) return ctx->virt;

    uint64_t cr3;
    __asm__ ("mov %%cr3, %0" : "=r" (cr3));
    return (uint64_t *) s_phys_to_virt(cr3);
}

static uint64_t *s_virt_entry_ptr_to_table_ptr(uint64_t *entry) {
    uint64_t table_phys = *entry & ENTRY_PHYS_ADDR_MASK;
    return (uint64_t *) s_phys_to_virt(table_phys);
}

static uint64_t *s_virt_get_entry_ptr(uint64_t *table, uint64_t index, bool create_if_missing, uint64_t flags) {
    uint64_t entry = table[index];
    if (entry == 0) {
        if (!create_if_missing) return NULL;

        uint64_t phys = phys_alloc();
        if (unlikely(phys == 0)) {
            print_stacktrace();
            kerror("Failed to allocate a paging structure!\n");
            hcf();
        }

        memset((void *) s_phys_to_virt(phys), 0, VIRT_PAGE_SIZE);
        table[index] = phys | flags | VF_PD_PRESENT;
    }

    return &table[index];
}

static uint64_t *s_virt_get_pte(virt_ctx_t *ctx, uint64_t addr, bool is_big_page) {
    uint64_t *pml4 = s_virt_get_top_level(ctx);
    uint64_t *pml4e = s_virt_get_entry_ptr(pml4, (addr >> VIRT_PML4_INDEX_OFFSET) & VIRT_INDEX_MASK, false, 0);
    if (!pml4e) return NULL;

    uint64_t *pdpt = s_virt_entry_ptr_to_table_ptr(pml4e);
    uint64_t *pdpte = s_virt_get_entry_ptr(pdpt, (addr >> VIRT_PDPT_INDEX_OFFSET) & VIRT_INDEX_MASK, false, 0);
    if (!pdpte) return NULL;

    uint64_t *pd = s_virt_entry_ptr_to_table_ptr(pdpte);
    uint64_t *pde = s_virt_get_entry_ptr(pd, (addr >> VIRT_PD_INDEX_OFFSET) & VIRT_INDEX_MASK, false, 0);
    if (!pde) return NULL;

    if (is_big_page) return pde;
    if (*pde & VF_PD_BIG_PAGE) return pde;

    uint64_t *pt = s_virt_entry_ptr_to_table_ptr(pde);
    return &pt[(addr >> VIRT_PT_INDEX_OFFSET) & VIRT_INDEX_MASK];
}

static uint64_t *s_virt_get_or_create_pte(virt_ctx_t *ctx, uint64_t addr, bool is_user, bool is_big_page) {
    uint64_t flags = (is_user ? VF_PD_USER : 0) | VF_PD_PRESENT;

    uint64_t *pml4 = s_virt_get_top_level(ctx);
    uint64_t *pml4e = s_virt_get_entry_ptr(pml4, (addr >> VIRT_PML4_INDEX_OFFSET) & VIRT_INDEX_MASK, true, flags | VF_PD_WRITABLE);
    if (unlikely(!pml4e)) return NULL;

    uint64_t *pdpt = s_virt_entry_ptr_to_table_ptr(pml4e);
    uint64_t *pdpte = s_virt_get_entry_ptr(pdpt, (addr >> VIRT_PDPT_INDEX_OFFSET) & VIRT_INDEX_MASK, true, flags | VF_PD_WRITABLE);
    if (unlikely(!pdpte)) return NULL;

    uint64_t *pd = s_virt_entry_ptr_to_table_ptr(pdpte);
    uint64_t *pde = s_virt_get_entry_ptr(pd, (addr >> VIRT_PD_INDEX_OFFSET) & VIRT_INDEX_MASK, true, flags | VF_PD_WRITABLE);
    if (unlikely(!pde)) return NULL;

    if (is_big_page) return pde;
    if (*pde & VF_PD_BIG_PAGE) return pde;

    uint64_t *pt = s_virt_entry_ptr_to_table_ptr(pde);
    return &pt[(addr >> VIRT_PT_INDEX_OFFSET) & VIRT_INDEX_MASK];
}

static bool s_virt_is_address_mapped(virt_ctx_t *ctx, uint64_t addr) {
    uint64_t *entry = s_virt_get_pte(ctx, addr, false);
    if (entry == NULL) return false;
    return *entry != 0;
}

void virt_init(void) {
    s_virt_hhdm_offset = limine_data_hhdm_get_offset();

    uint64_t kernel_pdpt_phys = phys_alloc();
    if (unlikely(!kernel_pdpt_phys)) {
        kerror("Failed to allocate the kernel PDPT!\n");
        hcf();
    }

    s_kernel_pdpt_virt = s_phys_to_virt(kernel_pdpt_phys);
    virt_ctx_t *ctx = (virt_ctx_t *) s_phys_to_virt(phys_alloc());
    ctx->phys = phys_alloc();
    ctx->virt = (uint64_t *) s_phys_to_virt(ctx->phys);

    struct limine_memmap_response *memmap = limine_data_get_memmap_response();
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
            case LIMINE_MEMMAP_RESERVED_MAPPED:
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_ACPI_NVS:
                virt_map(ctx, entry->base, entry->base + s_virt_hhdm_offset, ceilchunku64(entry->length, VIRT_PAGE_SIZE), VF_PT_PRESENT | VF_PT_WRITABLE);
                break;
        }
    }
    struct limine_framebuffer *framebuffer = limine_data_get_framebuffer_response()->framebuffers[0];
    virt_map(ctx, (uint64_t) framebuffer->address - s_virt_hhdm_offset, (uint64_t) framebuffer->address, framebuffer->width * framebuffer->height * framebuffer->bpp / BITS_PER_BYTE, VF_PT_PRESENT | VF_PT_WRITABLE);
    ktrace("Added HHDM map.\n");

    struct limine_executable_address_response *executable_address = limine_data_get_executable_address_response();
    virt_map(ctx, executable_address->physical_base, executable_address->virtual_base, (uint64_t) &kernel_end - (uint64_t) &kernel_start, VF_PT_PRESENT | VF_PT_WRITABLE);
    ktrace("Added kernel map.\n");

    virt_ctx_use(ctx);
}

virt_ctx_t *virt_ctx_new(void) {
    virt_ctx_t *ctx = (virt_ctx_t *) virt_alloc(NULL, VIRT_PAGE_SIZE);
    ctx->phys = phys_alloc();
    if (unlikely(!ctx->phys)) {
        kerror("Failed to allocate the PML4!\n");
        hcf();
    }

    ctx->virt = (uint64_t *) s_phys_to_virt(ctx->phys);
    ctx->virt[VIRT_ENTRIES_PER_STRUCT - 1] = s_virt_to_phys(s_kernel_pdpt_virt) | VF_PD_WRITABLE | VF_PD_PRESENT;

    return ctx;
}

void virt_ctx_use(virt_ctx_t *ctx) {
    uint64_t phys = ctx->phys;
    __asm__ volatile("mov %0, %%cr3" :: "r" (phys) : "memory");
}

uint64_t virt_alloc(virt_ctx_t *ctx, size_t size) {
    for (size_t addr = s_virt_hhdm_offset + VIRT_PAGE_SIZE; addr < 0xffffffffffff0000ull; addr += VIRT_PAGE_SIZE) {
        bool found = true;
        for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
            if (likely(s_virt_is_address_mapped(ctx, addr + i))) {
                found = false;
                break;
            }
        }

        if (unlikely(found)) {
            for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
                uint64_t phys = phys_alloc();
                if (unlikely(!phys)) {
                    kerror("Failed to allocate physical memory for a virt_alloc!\n");
                    hcf();
                }

                virt_map(ctx, phys, addr + i, VIRT_PAGE_SIZE, VF_PT_PRESENT | VF_PT_WRITABLE);
            }

            return addr;
        }
    }

    return 0;
}

uint64_t virt_user_alloc(virt_ctx_t *ctx, size_t size) {
    for (size_t addr = 0x100000; addr < s_virt_hhdm_offset; addr += VIRT_PAGE_SIZE) {
        bool found = true;
        for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
            if (likely(s_virt_is_address_mapped(ctx, addr + i))) {
                found = false;
                break;
            }
        }

        if (unlikely(found)) {
            for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
                uint64_t phys = phys_alloc();
                if (unlikely(!phys)) {
                    kerror("Failed to allocate physical memory for a virt_alloc!\n");
                    hcf();
                }

                virt_map_user(ctx, phys, addr + i, VIRT_PAGE_SIZE, VF_PT_PRESENT | VF_PT_USER | VF_PT_WRITABLE);
            }

            return addr;
        }
    }

    return 0;
}

void virt_free(virt_ctx_t *ctx, uint64_t addr, size_t size) {
}

void virt_map(virt_ctx_t *ctx, uint64_t phys_addr, uint64_t virt_addr, size_t size, uint64_t flags) {
    bool is_big_page = !!(flags & VF_PD_BIG_PAGE);
    uint64_t step = is_big_page ? VIRT_PAGE_SIZE_BIG : VIRT_PAGE_SIZE;

    for (size_t i = 0; i < size; i += step) {
        uint64_t curr_virt_addr = virt_addr + i;
        uint64_t *entry = s_virt_get_or_create_pte(ctx, curr_virt_addr, false, is_big_page);
        *entry = (phys_addr + i) | flags;
        if (ctx == NULL) {
            __asm__ volatile("invlpg (%0)" :: "r" (curr_virt_addr));
        }
    }
}

void virt_map_user(virt_ctx_t *ctx, uint64_t phys_addr, uint64_t virt_addr, size_t size, uint64_t flags) {
    bool is_big_page = !!(flags & VF_PD_BIG_PAGE);
    uint64_t step = is_big_page ? VIRT_PAGE_SIZE_BIG : VIRT_PAGE_SIZE;

    for (size_t i = 0; i < size; i += step) {
        uint64_t *entry = s_virt_get_or_create_pte(ctx, virt_addr + i, true, is_big_page);
        *entry = phys_addr | flags;
    }
}

void virt_unmap(virt_ctx_t *ctx, uint64_t addr, size_t size) {
    for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
        uint64_t curr_virt_addr = addr + i;
        uint64_t *entry = s_virt_get_pte(ctx, curr_virt_addr, false);
        if (entry) {
            *entry = 0;
            if (ctx == NULL) {
                __asm__ volatile("invlpg (%0)" :: "r" (curr_virt_addr));
            }
        }
    }
}

uint64_t virt_map_anywhere(virt_ctx_t *ctx, uint64_t phys_addr, size_t size, uint64_t flags) {
    for (size_t addr = s_virt_hhdm_offset + VIRT_PAGE_SIZE; addr < 0xffffffffffff0000ull; addr += VIRT_PAGE_SIZE) {
        bool found = true;
        for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
            if (s_virt_is_address_mapped(ctx, addr + i)) {
                found = false;
                break;
            }
        }

        if (found) {
            for (size_t i = 0; i < size; i += VIRT_PAGE_SIZE) {
                uint64_t curr_virt_addr = addr + i;
                uint64_t *entry = s_virt_get_or_create_pte(ctx, curr_virt_addr, false, false);
                *entry = (phys_addr + i) | flags;
                if (ctx == NULL) {
                    __asm__ volatile("invlpg (%0)" :: "r" (curr_virt_addr));
                }
            }

            return addr;
        }
    }

    return 0;
}

uint64_t virt_get_physical_address(virt_ctx_t *ctx, uint64_t virt_addr) {
    uint64_t *entry = s_virt_get_pte(ctx, virt_addr, false);
    if (entry == NULL) return 0;
    if (!(*entry & VF_PT_PRESENT)) return 0;
    return *entry & ENTRY_PHYS_ADDR_MASK;
}
