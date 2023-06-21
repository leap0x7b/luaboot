#include <stdint.h>
#include <stddef.h>
#include <luaboot/stdlib.h>

int mblen(const char *str, size_t size) {
    if (str == NULL) return 0;

    const char *len = str + size;
    int ret = 0;

    while (str < len && *str) {
        if ((*str & 128) != 0) {
            if ((*str & 32) == 0)
                str++;
            else if ((*str & 16) == 0)
                str += 2;
            else if ((*str & 8) == 0)
                str += 3;
        }
        ret++;
        str++;
    }

    return ret;
}

int mbtowc(wchar_t *dest, const char *src, size_t size) {
    if (src == NULL) return 0;

    wchar_t arg;
    int ret = 1;

    arg = (wchar_t)*src;
    if ((*src & 128) != 0) {
        if ((*src & 32) == 0 && size > 0) {
            arg = ((*src & 0x1F) << 6) | (*(src + 1) & 0x3F);
            ret = 2;
        } else if ((*src & 16) == 0 && size > 1) {
            arg = ((*src & 0xF) << 12) | ((*(src + 1) & 0x3F) << 6)|(*(src + 2) & 0x3F);
            ret = 3;
        } else if ((*src & 8) == 0 && size > 2) {
            arg = ((*src & 0x7) << 18) | ((*(src + 1) & 0x3F) << 12) | ((*(src + 2) & 0x3F) << 6) | (*(src+3) & 0x3F);
            ret = 4;
        } else return -1;
    }

    if (dest) *dest = arg;
    return ret;
}

int wctomb(char *dest, wchar_t wc) {
    int ret = 0;

    if (wc < 0x80) {
        *dest = wc;
        ret = 1;
    } else if (wc < 0x800) {
        *dest = ((wc >> 6) & 0x1F) | 0xC0;
        *(dest + 1) = (wc & 0x3F) | 0x80;
        ret = 2;
    } else {
        *dest = ((wc >> 12) & 0x0F) | 0xE0;
        *(dest + 1)= ((wc >> 6) & 0x3F) | 0x80;
        *(dest + 2)= (wc & 0x3F) | 0x80;
        ret = 3;
    }

    return ret;
}

size_t mbstowcs (wchar_t *dest, const char *src, size_t size) {
    if (src == NULL) return 0;

    int ret;
    wchar_t *orig = dest;

    while (*src) {
        ret = mbtowc(dest, src, size - (size_t)(dest - orig));
        if (ret < 0) return (size_t)-1;
        src++;
        dest += ret;
    }

    *dest = 0;
    return (size_t)(dest - orig);
}

size_t wcstombs(char *dest, const wchar_t *src, size_t size) {
    if (src == NULL) return 0;

    int ret;
    char *orig = dest;

    while (*src && ((size_t)(dest - orig + 4) < size)) {
        ret = wctomb(dest, *src);
        if (ret < 0) return (size_t)-1;
        src++;
        dest += ret;
    }
    *dest = 0;
    return (size_t)(dest - orig);
}
