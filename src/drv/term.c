#include "term.h"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_HEX_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 1
#define NANOPRINTF_USE_FLOAT_SINGLE_PRECISION 0
#define NANOPRINTF_USE_VISIBILITY_STATIC 1
#define NANOPRINTF_IMPLEMENTATION

#include <stdarg.h>

#include <nanoprintf.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include "drv/rs232.h"
#include "limine_data.h"

#define TEXT_BUFFER_SIZE 4096

static struct {
    uint32_t fg;
    uint32_t bg;
} s_term_defaults = {
    .fg = 0xffffff,
    .bg = 0x000000,
};

static struct flanterm_context *s_term;

void term_init() {
    struct limine_framebuffer *fb = limine_data_get_framebuffer_response()->framebuffers[0];
    struct limine_flanterm_fb_init_params *flanterm_params = limine_data_get_flanterm_fb_init_params_response()->entries[0];

    s_term = flanterm_fb_init(
        NULL, NULL,
        fb->address, fb->width, fb->height, fb->pitch,
        fb->red_mask_size, fb->red_mask_shift,
        fb->green_mask_size, fb->green_mask_shift,
        fb->blue_mask_size, fb->blue_mask_shift,
        flanterm_params->canvas, flanterm_params->ansi_colours, flanterm_params->ansi_bright_colours,
        &s_term_defaults.bg, &s_term_defaults.fg, &flanterm_params->default_bg_bright, &flanterm_params->default_fg_bright,
        flanterm_params->font, flanterm_params->font_width, flanterm_params->font_height, flanterm_params->font_spacing,
        flanterm_params->font_scale_x, flanterm_params->font_scale_y,
        flanterm_params->margin, flanterm_params->rotation
    );

    flanterm_set_autoflush(s_term, false);

    serial_init(KERNEL_LOGGING_PORT, S_BAUD_115200);

    kprintf("\033[2J");
}

#define ANSI_SGR_FG_COLOR "\033[38;5;"
#define ANSI_SGR_END "m"
#define ANSI_SGR_FG_COLOR_RESET "\033[39;49m"

static const char *LOG_COLORS[] = {
    "7",
    "69",
    "221",
    "160",
};

static const char *LOG_LEVEL_NAMES[] = {
    "dbg",
    "inf",
    "wrn",
    "err",
};

size_t s_print_log_head(int log_level, const char *file, int line) {
    if (log_level < LL_DEBUG || log_level > LL_ERROR) {
        kerror("Invalid log level %d in %s:%d, rewriting to LL_ERROR.\n", log_level, file, line);
        log_level = LL_ERROR;
    }

    return kprintf("[%s%s%s%s%s] [%s:%3d] ", ANSI_SGR_FG_COLOR, LOG_COLORS[log_level], ANSI_SGR_END, LOG_LEVEL_NAMES[log_level], ANSI_SGR_FG_COLOR_RESET, file, line);
}

static void s_putchar(int wide_c, void *ctx) {
    (void) ctx;

    char c = (char) wide_c;
    if (c == '\n') {
        flanterm_write(s_term, "\r", 1);
    }

    flanterm_write(s_term, &c, 1);
    serial_write(KERNEL_LOGGING_PORT, &c, 1);
}

size_t kvprintf(const char *fmt, va_list args) {
    size_t size = npf_vpprintf(s_putchar, NULL, fmt, args);
    flanterm_flush(s_term);
    return size;
}

size_t kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = kvprintf(fmt, args);
    va_end(args);

    return size;
}

size_t kvlog(int log_level, const char *file, int line, const char *fmt, va_list args) {
    size_t size = s_print_log_head(log_level, file, line);
    size += kvprintf(fmt, args);

    return size;
}

size_t klog(int log_level, const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = kvlog(log_level, file, line, fmt, args);
    va_end(args);
    return size;
}
