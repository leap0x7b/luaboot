#include <stdint.h>
#include <stdbool.h>
#include <luaboot/stdlib.h>
#include <luaboot/printf.h>
#include <luaboot/efi.h>

EFI_HANDLE IM;
EFI_SYSTEM_TABLE *ST;
EFI_BOOT_SERVICES *BS;
EFI_RUNTIME_SERVICES *RT;
EFI_LOADED_IMAGE_PROTOCOL *LIP;
EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

void efi_init(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table) {
    IM = handle;
    ST = system_table;
    BS = system_table->BootServices;
    RT = system_table->RuntimeServices;
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    BS->HandleProtocol(handle, &lip_guid, (void **)&LIP);
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    BS->LocateProtocol(&gop_guid, NULL, (void **)&GOP);
}

void efi_console_reset(void) {
    ST->ConOut->Reset(ST->ConOut, false);
    ST->ConIn->Reset(ST->ConIn, false);
}

void efi_console_set_attribute(uint64_t attribute) {
    ST->ConOut->SetAttribute(ST->ConOut, attribute);
}

void efi_console_clear(void) {
    ST->ConOut->ClearScreen(ST->ConOut);
}

void efi_console_write(uint16_t *s) {
    ST->ConOut->OutputString(ST->ConOut, s);
}

static void _printf_callback(char c, void *) {
    wchar_t *dest = malloc(4);
    mbstowcs(dest, &c, 4);
    ST->ConOut->OutputString(ST->ConOut, dest);
}

int efi_console_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfctprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int efi_console_vprintf(const char *format, va_list args) {
    return vfctprintf(&_printf_callback, NULL, format, args);
}

void efi_console_show_cursor(void) {
    ST->ConOut->EnableCursor(ST->ConOut, true);
}

void efi_console_hide_cursor(void) {
    ST->ConOut->EnableCursor(ST->ConOut, false);
}

EFI_INPUT_KEY efi_console_read_key(void) {
    EFI_INPUT_KEY key = {};
    uint64_t key_event = 0;

    BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &key_event);
    ST->ConIn->ReadKeyStroke(ST->ConIn, &key);

    return key;
}
