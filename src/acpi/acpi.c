#include "acpi.h"

#include <uacpi/acpi.h>
#include <uacpi/event.h>
#include <uacpi/tables.h>

#include "drv/term.h"
#include "mem/virt.h"
#include "util.h"

static void *s_acpi_early_table_buffer = NULL;
static uint64_t s_local_apic_address_phys = 0;

void acpi_init(void) {
    s_acpi_early_table_buffer = (void *) virt_alloc(NULL, VIRT_PAGE_SIZE);
    uacpi_status err = uacpi_setup_early_table_access(s_acpi_early_table_buffer, VIRT_PAGE_SIZE);
    if (err != UACPI_STATUS_OK) {
        kerror("Failed to initialize uACPI: %s\n", uacpi_status_to_string(err));
        hcf();
    }

    uacpi_table madt_table;
    uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &madt_table);
    ktrace("MADT @ %0p\n", madt_table.hdr);

    struct acpi_madt *madt = (struct acpi_madt *) madt_table.ptr;
    s_local_apic_address_phys = (uint64_t) madt->local_interrupt_controller_address;

    size_t offset = 0x2c;
    size_t i = 0;
    while (offset < madt_table.hdr->length) {
        struct acpi_entry_hdr *entry_header = madt_table.ptr + offset;

#if EXTREMELY_VERBOSE_LOGGING
        kdebug("MADT entry %llu type %d\n", i++, entry_header->type);
#endif

        if (entry_header->type == ACPI_MADT_ENTRY_TYPE_LAPIC_ADDRESS_OVERRIDE) {
            struct acpi_madt_lapic_address_override *entry = (struct acpi_madt_lapic_address_override *) entry_header;
            s_local_apic_address_phys = entry->address;
        }

        offset += entry_header->length;
    }
}

uint64_t acpi_get_lapic_address(void) {
    return s_local_apic_address_phys;
}
