#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <efi.h>

extern EFI_HANDLE IM;
extern EFI_SYSTEM_TABLE *ST;
extern EFI_BOOT_SERVICES *BS;
extern EFI_LOADED_IMAGE_PROTOCOL *LIP;

void efi_init(EFI_HANDLE handle, EFI_SYSTEM_TABLE *st);

void efi_console_reset(void);
void efi_console_set_attribute(uint64_t attribute);
void efi_console_clear(void);
void efi_console_show_cursor(void);
void efi_console_hide_cursor(void);
void efi_console_write(uint16_t *s);
int efi_console_printf(const char *format, ...);
int efi_console_vprintf(const char *format, va_list args);

EFI_INPUT_KEY efi_console_read_key(void);
