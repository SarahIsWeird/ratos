#include <uacpi/kernel_api.h>

#include <stdarg.h>

#include "drv/term.h"
#include "limine_data.h"
#include "mem/virt.h"
#include "util.h"

int s_kernel_api_convert_log_level(uacpi_log_level log_level) {
    switch (log_level) {
        case UACPI_LOG_DEBUG:
            return LL_DEBUG;
        case UACPI_LOG_TRACE:
            return LL_TRACE;
        case UACPI_LOG_INFO:
            return LL_INFO;
        case UACPI_LOG_WARN:
            return LL_WARN;
        case UACPI_LOG_ERROR:
            return LL_ERROR;
        default:
            return LL_ERROR;
    }
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
    *out_rsdp_address = (uint64_t) limine_data_get_rsdp_phys();
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
    uint64_t phys_base = floorchunku64(addr, VIRT_PAGE_SIZE);
    uint64_t offset_within_page = addr - phys_base;
    uint64_t actual_len = ceilchunku64(len + offset_within_page, VIRT_PAGE_SIZE);

    uint64_t virt_addr = virt_map_anywhere(NULL, phys_base, actual_len, VF_PT_PRESENT | VF_PT_WRITABLE);

    return (void *) (virt_addr + offset_within_page);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
    uint64_t virt_base = floorchunku64((uint64_t) addr, VIRT_PAGE_SIZE);
    uint64_t offset_within_page = (uint64_t) addr - virt_base;
    uint64_t actual_len = ceilchunku64(len + offset_within_page, VIRT_PAGE_SIZE);

    virt_unmap(NULL, virt_base, actual_len);
}

void uacpi_kernel_log(uacpi_log_level log_level, const uacpi_char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    uacpi_kernel_vlog(log_level, fmt, args);
    va_end(args);
}

void uacpi_kernel_vlog(uacpi_log_level log_level, const uacpi_char* fmt, uacpi_va_list args) {
    kvlog(s_kernel_api_convert_log_level(log_level), "uACPI", -1, fmt, args);
}
