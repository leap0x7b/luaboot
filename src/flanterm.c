#include <luaboot/efi.h>
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <luaboot/nanoprintf_config.h>
#include <luaboot/nanoprintf.h>

struct flanterm_context *flanterm;

void flanterm_init(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    flanterm = flanterm_fb_simple_init(
        (uint32_t *)gop->Mode->FrameBufferBase,
        gop->Mode->Info->HorizontalResolution,
        gop->Mode->Info->VerticalResolution,
        gop->Mode->Info->HorizontalResolution * 4
    );
}

static void _printf_callback(int c, void *) {
    flanterm_write(flanterm, &c, 1);
}

int flanterm_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int flanterm_vprintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
