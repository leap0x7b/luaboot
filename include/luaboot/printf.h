#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

int snprintf(char* buffer, size_t count, const char* format, ...);
int vsnprintf(char* buffer, size_t count, const char* format, va_list args);
int efi_console_printf(const char *format, ...);
int efi_console_vprintf(const char *format, va_list args);
