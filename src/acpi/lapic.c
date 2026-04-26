#include "lapic.h"

#include "acpi/acpi.h"
#include "drv/term.h"
#include "mem/virt.h"
#include "util.h"

#define MSR_APIC_BASE 0x1b

#define MSR_APIC_BASE_FLAGS_IS_BOOTSTRAP_PROCESSOR  0x100
#define MSR_APIC_BASE_FLAGS_ENABLED                 0x800

void lapic_init(void) {
    uint64_t local_apic_address_phys = acpi_get_lapic_address();
    kinfo("The local APIC lies at the physical address %0p, ", local_apic_address_phys);
    uint64_t *local_apic_address = (uint64_t *) virt_map_anywhere(NULL, local_apic_address_phys, VIRT_PAGE_SIZE, VF_PT_PRESENT | VF_PT_WRITABLE | VF_PT_WRITE_THROUGH | VF_PT_CACHE_DISABLE);
    kprintf("and is mapped to %0p.\n", local_apic_address);

    uint64_t apic_base_msr = msr_read(MSR_APIC_BASE);
    kinfo("The APIC MSR says the APIC is located at %0p.\n", apic_base_msr);

    if (unlikely(!(apic_base_msr & MSR_APIC_BASE_FLAGS_ENABLED))) {
        kerror("The APIC was disabled! We cannot re-enable it.\n");
        hcf();
    }
}
