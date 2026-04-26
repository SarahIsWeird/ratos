#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

void acpi_init(void);

uint64_t acpi_get_lapic_address(void);

#endif /* ACPI_H */
