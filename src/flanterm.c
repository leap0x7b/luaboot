#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <luaboot/efi.h>
#include <luaboot/printf.h>
#include <stdint.h>
#include <stdlib.h>

struct flanterm_context *flanterm;

static uint16_t linear_masks_to_bpp(uint32_t red_mask, uint32_t green_mask,
                                    uint32_t blue_mask, uint32_t alpha_mask) {
    uint32_t compound_mask = red_mask | green_mask | blue_mask | alpha_mask;
    uint16_t ret = 32;
    while ((compound_mask & (1 << 31)) == 0) {
        ret--;
        compound_mask <<= 1;
    }
    return ret;
}

static void linear_mask_to_mask_shift(uint8_t *mask, uint8_t *shift, uint32_t linear_mask) {
    *shift = 0;
    while ((linear_mask & 1) == 0) {
        (*shift)++;
        linear_mask >>= 1;
    }
    *mask = 0;
    while ((linear_mask & 1) == 1) {
        (*mask)++;
        linear_mask >>= 1;
    }
}

static void _free(void *ptr, uint64_t) {
    free(ptr);
}

void flanterm_init(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    int bpp;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;

    switch (gop->Mode->Info->PixelFormat) {
        case PixelBlueGreenRedReserved8BitPerColor:
            bpp = 32;
            red_mask_size = 8;
            red_mask_shift = 16;
            green_mask_size = 8;
            green_mask_shift = 8;
            blue_mask_size = 8;
            blue_mask_shift = 0;
            break;
        case PixelRedGreenBlueReserved8BitPerColor:
            bpp = 32;
            red_mask_size = 8;
            red_mask_shift = 0;
            green_mask_size = 8;
            green_mask_shift = 8;
            blue_mask_size = 8;
            blue_mask_shift = 16;
            break;
        case PixelBitMask:
            bpp = linear_masks_to_bpp(gop->Mode->Info->PixelInformation.RedMask,
                                      gop->Mode->Info->PixelInformation.GreenMask,
                                      gop->Mode->Info->PixelInformation.BlueMask,
                                      gop->Mode->Info->PixelInformation.ReservedMask);
            linear_mask_to_mask_shift(&red_mask_size,
                                      &red_mask_shift,
                                      gop->Mode->Info->PixelInformation.RedMask);
            linear_mask_to_mask_shift(&green_mask_size,
                                      &green_mask_shift,
                                      gop->Mode->Info->PixelInformation.GreenMask);
            linear_mask_to_mask_shift(&blue_mask_size,
                                      &blue_mask_shift,
                                      gop->Mode->Info->PixelInformation.BlueMask);
            break;
        default:
            // Just assume its an RGB framebuffer with 8 bits per color
            bpp = 32;
            red_mask_size = 8;
            red_mask_shift = 0;
            green_mask_size = 8;
            green_mask_shift = 8;
            blue_mask_size = 8;
            blue_mask_shift = 16;
            break;
    }

    struct flanterm_context *ft_ctx = flanterm_fb_init(
        malloc,
        _free,
        (uint32_t *)gop->Mode->FrameBufferBase,
        gop->Mode->Info->HorizontalResolution,
        gop->Mode->Info->VerticalResolution,
        gop->Mode->Info->PixelsPerScanLine * (bpp / 8),
        red_mask_size,
        red_mask_shift,
        green_mask_size,
        green_mask_shift,
        blue_mask_size,
        blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0);
}

static void _printf_callback(char c, void *) {
    flanterm_write(flanterm, &c, 1);
}

int flanterm_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfctprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int flanterm_vprintf(const char *format, va_list args) {
    return vfctprintf(&_printf_callback, NULL, format, args);
}
