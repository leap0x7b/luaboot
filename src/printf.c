#include <stdint.h>
#include <luaboot/stdlib.h>
#include <luaboot/printf.h>
#include <luaboot/nanoprintf_config.h>
#define NANOPRINTF_IMPLEMENTATION
#include <luaboot/nanoprintf.h>
#include <tinyefi/tinyefi.h>

int snprintf(char* buffer, size_t count, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vsnprintf(buffer, count, format, args);
    va_end(args);
    return ret;
}

int vsnprintf(char* buffer, size_t count, const char* format, va_list args) {
    return npf_vsnprintf(buffer, count, format, args);
}

int efi_console_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    wchar_t dest[512];
    char tmp[512];

    int ret = npf_vsnprintf(tmp, 512, format, args);
    mbstowcs(dest, tmp, 511);
    efi_console_write((wchar_t *)&dest);

    va_end(args);
    return ret;
}

int efi_console_vprintf(const char *format, va_list args) {
    wchar_t dest[1024];
    char tmp[1024];

    int ret = npf_vsnprintf(tmp, 1024, format, args);
    mbstowcs(dest, tmp, 1024);
    efi_console_write((wchar_t *)&dest);

    return ret;
}
