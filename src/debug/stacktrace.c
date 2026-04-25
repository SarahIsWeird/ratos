#include "stacktrace.h"

#include <stddef.h>
#include <stdint.h>

#include "drv/term.h"
#include "limine_data.h"
#include "string.h"
#include "util.h"

#define STACKTRACE_MAX_RECURSION 64
#define INDENT "        "

uint64_t s_pointer_to_number(const char *str) {
    uint64_t n = 0;

    for (int i = 0; i < 16; i++) {
        char c = str[i];

        n <<= 4;

        if (c >= '0' && c <= '9') {
            n += (uint64_t) (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            n += (uint64_t) (c - 'a') + 0xa;
        } else if (c >= 'A' && c <= 'F') {
            n += (uint64_t) (c - 'A') + 0xa;
        }
    }

    return n;
}

static bool s_was_common_handler = false;

void s_print_function_name(uint64_t return_address, bool skip_printing) {
    struct limine_module_response *modules = limine_data_get_module_response();
    struct limine_file *symbol_map_file = NULL;
    for (size_t i = 0; i < modules->module_count; i++) {
        if (!strcmp(modules->modules[i]->string, "debug symbol map")) {
            symbol_map_file = modules->modules[i];
            break;
        }
    }

    if (symbol_map_file == NULL) {
        kerror("failed to find the symbol map file?\n");
        hcf();
    }

    static char function_name[64] = "???";

    const char *data = (const char *) symbol_map_file->address;
    char *curr_data = data;
    while ((curr_data != NULL) && (strchr(curr_data, '\n') - data > 10) && ((curr_data - data) < (ptrdiff_t) symbol_map_file->size)) {
        uint64_t addr = s_pointer_to_number(curr_data);
        if (addr > return_address) break;

        char *end_of_line = strchr(curr_data, '\n');
        *end_of_line = 0;
        strncpy(function_name, curr_data + 17, 63);
        *end_of_line = '\n';
        curr_data = end_of_line + 1;
    }

    s_was_common_handler = !strncmp("idt_common_handler", function_name, 63);

    if (!skip_printing) {
        kprintf("%s", function_name);
    }
}

__attribute__((noinline)) void print_stacktrace(void) {
    uint64_t current_instruction_pointer;
    uint64_t *current_base_pointer;
    __asm__ ("mov %%rbp, %0;"
             "lea 0(%%rip), %1;"
        : "=r" (current_base_pointer), "=r" (current_instruction_pointer));
    
    size_t skips = 3;

    for (size_t i = 0; current_base_pointer != NULL && i < STACKTRACE_MAX_RECURSION; i++) {
        uint64_t rip = *(current_base_pointer + 1);
        if (rip == 0) {
            kprintf(INDENT "at _start\n");
            break;
        }

        if (skips == 0) {
            kprintf(INDENT "at %0p ", rip);
        }

        s_print_function_name(rip, skips != 0);

        if (s_was_common_handler) {
            if (skips == 0) kprintf("\n");

            kprintf("%sat %0p ", INDENT, *(current_base_pointer + 19));
            s_print_function_name(*(current_base_pointer + 19), false);

            if (skips != 0) kprintf("\n");
            s_was_common_handler = true;
        }

        if (skips == 0) {
            kprintf("\n");
        } else {
            skips--;
        }

        if (s_was_common_handler) {
            current_base_pointer = (uint64_t *) *(current_base_pointer + 8);
        } else {
            current_base_pointer = (uint64_t *) *current_base_pointer;
        }
    }
}
