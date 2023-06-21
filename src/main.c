#include <stdbool.h>
#include <luaboot/e9.h>
#include <luaboot/printf.h>
#include <tinyefi/tinyefi.h>

EfiStatus main(EfiHandle image_handle, EfiSystemTable *st) {
    efi_init(image_handle, st);

    efi_console_clear();
    efi_console_reset();

    e9_write("[luaboot:main] Hello World!\n");

    efi_console_write(L"Hello, World!");
    efi_console_printf("Hello, %s!", "World");

    while (true) {
        asm volatile ("hlt");
    }

    return EFI_SUCCESS;
}
