#pragma once
#include <stdarg.h>

void e9_write_char(char c);
void e9_write(const char *string);
int e9_printf(const char *format, ...);
int e9_vprintf(const char *format, va_list args);
