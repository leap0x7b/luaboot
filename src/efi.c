#include <stdint.h>
#include <stdbool.h>
#include <efi.h>

EFI_HANDLE IM;
EFI_SYSTEM_TABLE *ST;
EFI_BOOT_SERVICES *BS;
EFI_LOADED_IMAGE_PROTOCOL *LIP;

void efi_init(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table) {
    IM = handle;
    ST = system_table;
    BS = system_table->BootServices;
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    BS->HandleProtocol(handle, &lip_guid, (void **)&LIP);
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
