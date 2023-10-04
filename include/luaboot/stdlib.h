#pragma once
#include <stddef.h>
#include <gdtoa.h>
#include <luaboot/efi.h>

#define EXIT_SUCCESS EFI_SUCCESS
#define EXIT_FAILURE EFI_ABORTED

int atoi(const char *str);
int64_t atol(const char *str);
int64_t strtol(const char *str, char **dest, int base);
const char *getenv(const char *env);
int system(const char *cmd);
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
int abs(int i);
int mblen(const char *str, size_t size);
int mbtowc(wchar_t *dest, const char *src, size_t size);
int wctomb(char *dest, wchar_t wc);
size_t mbstowcs(wchar_t *dest, const char *src, size_t size);
size_t wcstombs(char *dest, const wchar_t *src, size_t size);
void exit(int) __attribute__((noreturn));
void abort(void) __attribute__((noreturn));
