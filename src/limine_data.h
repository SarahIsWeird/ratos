#ifndef LIMINE_DATA_H
#define LIMINE_DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <limine.h>

bool limine_data_bare_minimum_present(void);
bool limine_data_wanted_responses_present(void);

uint64_t limine_data_hhdm_get_offset(void);
uint64_t limine_data_paging_get_mode(void);

struct limine_flanterm_fb_init_params_response *limine_data_get_flanterm_fb_init_params_response(void);
struct limine_framebuffer_response *limine_data_get_framebuffer_response(void);
struct limine_memmap_response *limine_data_get_memmap_response(void);
struct limine_module_response *limine_data_get_module_response(void);
struct limine_executable_address_response *limine_data_get_executable_address_response(void);

#endif
