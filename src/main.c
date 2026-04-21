#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "cpuid.h"
#include "drv/term.h"
#include "mem/gdt.h"
#include "mem/phys.h"
#include "mem/virt.h"
#include "int/idt.h"
#include "string.h"
#include "util.h"

LIMINE_REQUEST uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);
LIMINE_REQUEST struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};
LIMINE_REQUEST struct limine_flanterm_fb_init_params_request flanterm_request = {
    .id = LIMINE_FLANTERM_FB_INIT_PARAMS_REQUEST_ID,
    .revision = 0,
};
LIMINE_REQUEST struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};
LIMINE_REQUEST struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST_ID,
    .revision = 1,
    .mode     = LIMINE_PAGING_MODE_X86_64_4LVL,
    .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .max_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
};
LIMINE_REQUEST struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hlt() {
    for (;;) {
        __asm__ ("hlt");
    }
}

void kmain(void) {
    __asm__ volatile("cli");

    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hlt();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hlt();
    }

    if (flanterm_request.response == NULL || flanterm_request.response->entry_count < 1) {
        hlt();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    struct limine_flanterm_fb_init_params *ft_params = flanterm_request.response->entries[0];

    term_init(fb, ft_params);

    kdebug("hello rats :3\n");

    gdt_init();
    kinfo("gdt inited\n");

    idt_init();
    kinfo("idt inited\n");

    phys_init(memmap_request, hhdm_request.response->offset);
    kinfo("phys inited\n");

    kinfo("cpuid max leaf:          %08x\n", cpuid_eax(CPUID_MAX_LEAF_AND_VENDOR_ID, 0));
    kinfo("cpuid max extended leaf: %08x\n", cpuid_eax(CPUID_MAX_EXTENDED_LEAF, 0));

    virt_init(memmap_request.response, hhdm_request.response->offset);

    uint64_t *foo = (uint64_t *) virt_alloc(NULL, 0x10000, VF_WRITABLE);
    *foo = 0xdeadbeefcafefe00;
    kdebug("*%llp=%llp\n", foo, *foo);

    while (1);

    hlt();
}
