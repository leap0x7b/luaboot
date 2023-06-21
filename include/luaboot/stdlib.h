#pragma once
#include <stddef.h>

int mblen(const char *str, size_t size);
int mbtowc(wchar_t *dest, const char *src, size_t size);
int wctomb(char *dest, wchar_t wc);
size_t mbstowcs (wchar_t *dest, const char *src, size_t size);
size_t wcstombs(char *dest, const wchar_t *src, size_t size);