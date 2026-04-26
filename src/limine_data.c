#include "limine_data.h"

#include "drv/term.h"
#include "mem/virt.h"
#include "util.h"

#define LIMINE_SECTION(suffix) __attribute__((used, section(".limine_requests" suffix))) static volatile
#define LIMINE_REQUEST LIMINE_SECTION("")

#define MODULE_COUNT 1

struct limine_internal_module debug_symbol_module = {
    .path = "rats/symbols.map",
    .string = "debug symbol map",
    .flags = LIMINE_INTERNAL_MODULE_REQUIRED,
};

struct limine_internal_module *modules[] = {
    &debug_symbol_module,
};

LIMINE_SECTION("_start") uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

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

LIMINE_REQUEST struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 1,
    .internal_module_count = MODULE_COUNT,
    .internal_modules = modules,
};

LIMINE_REQUEST struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
};

LIMINE_REQUEST struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
};

LIMINE_SECTION("_end") uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void *s_limine_data_rsdp_phys = NULL;

bool limine_data_bare_minimum_present(void) {
    if (unlikely(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)) {
        return false;
    }

    if (unlikely(framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)) {
        return false;
    }

    if (unlikely(flanterm_request.response == NULL || flanterm_request.response->entry_count < 1)) {
        return false;
    }

    return true;
}

bool limine_data_wanted_responses_present(void) {
    if (unlikely(hhdm_request.response == NULL)) {
        kerror("Limine didn't provide a HHDM response..?\n");
        return false;
    }

    if (unlikely(memmap_request.response == NULL || memmap_request.response->entry_count < 1)) {
        kerror("Limine didn't pass a memory map! :(\n");
        return false;
    }

    if (unlikely(paging_mode_request.response == NULL)) {
        kerror("Limine didn't provide a paging mode response!\n");
        return false;
    }

    if (unlikely(module_request.response == NULL || module_request.response->module_count < 1)) {
        kerror("Limine didn't load any modules!\n");
        return false;
    } else {
        kinfo("Limine loaded %llu modules:\n", module_request.response->module_count);
        for (size_t i = 0; i < module_request.response->module_count; i++) {
            struct limine_file *module = module_request.response->modules[i];
            kinfo("    %2llu: %s (%s) @ %0p (size %0p)\n", i, module->string, module->path, module->address, module->size);
        }
    }

    if (unlikely(executable_address_request.response == NULL)) {
        kerror("Limine didn't provide the executable addresses!\n");
        return false;
    }

    if (unlikely(rsdp_request.response == NULL)) {
        kerror("ACPI is unavailable! (RSDP response missing)\n");
        return false;
    }

    return true;
}

uint64_t limine_data_hhdm_get_offset(void) {
    return hhdm_request.response->offset;
}

struct limine_flanterm_fb_init_params_response *limine_data_get_flanterm_fb_init_params_response(void) {
    return flanterm_request.response;
}

struct limine_framebuffer_response *limine_data_get_framebuffer_response(void) {
    return framebuffer_request.response;
}

struct limine_memmap_response *limine_data_get_memmap_response(void) {
    return memmap_request.response;
}

uint64_t limine_data_paging_get_mode(void) {
    return paging_mode_request.response->mode;
}

struct limine_module_response *limine_data_get_module_response(void) {
    return module_request.response;
}

struct limine_executable_address_response *limine_data_get_executable_address_response(void) {
    return executable_address_request.response;
}

void *limine_data_get_rsdp_virt(void) {
    return rsdp_request.response->address;
}

void *limine_data_get_rsdp_phys(void) {
    if (s_limine_data_rsdp_phys == NULL) {
        s_limine_data_rsdp_phys = (void *) virt_get_physical_address(NULL, (uint64_t) limine_data_get_rsdp_virt());
    }

    return s_limine_data_rsdp_phys;
}
