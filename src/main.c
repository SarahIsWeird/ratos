#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "acpi/acpi.h"
#include "acpi/lapic.h"
#include "cpuid.h"
#include "debug/stacktrace.h"
#include "drv/term.h"
#include "limine_data.h"
#include "mem/gdt.h"
#include "mem/phys.h"
#include "mem/virt.h"
#include "int/idt.h"
#include "string.h"
#include "util.h"

void kmain(void) {
    __asm__ volatile("cli");

    if (!limine_data_bare_minimum_present()) {
        hcf();
    }

    term_init();

    if (!limine_data_wanted_responses_present()) {
        hcf();
    }

    kdebug("hello rats :3\n");

    gdt_init();
    kinfo("gdt inited\n");

    idt_init();
    kinfo("idt inited\n");

    phys_init();
    kinfo("phys inited\n");

    kdebug("cpuid max leaf:          %08x\n", cpuid_eax(CPUID_MAX_LEAF_AND_VENDOR_ID, 0));
    kdebug("cpuid max extended leaf: %08x\n", cpuid_eax(CPUID_MAX_EXTENDED_LEAF, 0));

    virt_init();
    kinfo("virt inited\n");

    acpi_init();
    kinfo("acpi inited\n");

    lapic_init();
    kinfo("lapic inited\n");

    while (1);

    hcf();
}
