#include <stddef.h>
#include <luaboot/e9.h>
#include <luaboot/string.h>
#include <luaboot/io.h>
#include <luaboot/nanoprintf_config.h>
#include <luaboot/nanoprintf.h>

void e9_write_char(char c) {
    io_outb(0xe9, c);
}

void e9_write(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        e9_write_char(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    e9_write_char(c);
}

int e9_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int e9_vprintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
