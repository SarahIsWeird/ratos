#ifndef TERM_H
#define TERM_H

#include <stddef.h>
#include <stdarg.h>

#include <limine.h>

#define LL_TRACE 0x00
#define LL_DEBUG 0x01
#define LL_INFO  0x02
#define LL_WARN  0x03
#define LL_ERROR 0x04

void term_init();

size_t kvprintf(const char *fmt, va_list args);
/* __attribute__((format(printf, 1, 2))) */ size_t kprintf(const char *fmt, ...);

size_t kvlog(int log_level, const char *file, int line, const char *fmt, va_list args);
/* __attribute__((format(printf, 4, 5))) */ size_t klog(int log_level, const char *file, int line, const char *fmt, ...);

#define ktrace(...) klog(LL_TRACE, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kdebug(...) klog(LL_DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kinfo(...)  klog(LL_INFO , __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kwarn(...)  klog(LL_WARN , __FILE_NAME__, __LINE__, __VA_ARGS__)
#define kerror(...) klog(LL_ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)

#endif /* TERM_H */
