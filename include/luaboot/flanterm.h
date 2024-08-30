#pragma once
#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <luaboot/efi.h>

extern struct flanterm_context *flanterm;

void flanterm_init(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);
int flanterm_printf(const char *format, ...);
int flanterm_vprintf(const char *format, va_list args);
