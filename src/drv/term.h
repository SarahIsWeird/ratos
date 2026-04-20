#ifndef TERM_H
#define TERM_H

#include <stddef.h>
#include <stdarg.h>

#include <limine.h>

#define LL_DEBUG 0x00
#define LL_INFO  0x01
#define LL_WARN  0x02
#define LL_ERROR 0x03

void term_init(struct limine_framebuffer *fb, struct limine_flanterm_fb_init_params *flanterm_params);

size_t kvprintf(const char *fmt, va_list args);
size_t kprintf(const char *fmt, ...);

size_t kvlog(int log_level, const char *file, int line, const char *fmt, va_list args);
size_t klog(int log_level, const char *file, int line, const char *fmt, ...);

#define kdebug(...) klog(LL_DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kinfo(...)  klog(LL_INFO , __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kwarn(...)  klog(LL_WARN , __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kerror(...) klog(LL_ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)

#endif /* TERM_H */
