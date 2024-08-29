#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <luaboot/efi.h>
#include <luaboot/e9.h>

int atoi(const char *str) {
    return (int)atol(str);
};

int64_t atol(const char *str) {
    int64_t sign = 1;

    if (!str || !*str)
        return 0;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    if (str[0] == '0') {
        if (str[1] == 'x')
            return strtol(str + 2, NULL, 16) * sign;
        if (str[1] >= '0' && str[1] <= '7')
            return strtol(str, NULL, 8) * sign;
    }

    return strtol(str, NULL, 10) * sign;
};

int64_t strtol(const char *str, char **dest, int base) {
    int64_t v = 0;
    int64_t sign = 1;

    if (!str || !*str)
        return 0;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (!(*str < '0' || (base < 10 && *str >= base + '0') || (base >= 10 && ((*str > '9' && *str < 'A') ||
            (*str > 'F' && *str < 'a') || *str > 'f')))) {
        v *= base;
        if (*str >= '0' && *str <= (base < 10 ? base + '0' : '9'))
            v += (*str) - '0';
        else if (base == 16 && *str >= 'a' && *str <= 'f')
            v += (*str) - 'a' + 10;
        else if (base == 16 && *str >= 'A' && *str <= 'F')
            v += (*str) - 'A' + 10;
        str++;
    }

    if (dest)
        *dest = (char *)str;

    return v * sign;
}

const char *getenv(const char *name) {
    char tmp[EFI_MAXIMUM_VARIABLE_SIZE], *ret;
    size_t len;

    wchar_t wcname[256];
    mbstowcs((wchar_t *)&wcname, name, 256);

    EFI_GUID guid = EFI_GLOBAL_VARIABLE;
    EFI_STATUS status = RT->GetVariable((wchar_t*)&wcname, &guid, NULL, &len, &tmp);
    if (EFI_ERROR(status) || len < 1 || !(ret = malloc((len) + 1))) {
        return NULL;
    }

    memcpy(ret, tmp, len);
    ret[len] = 0;

    return ret;
}

int system(const char *cmd) {
    e9_printf("$ %s\ncan't do system()\n", cmd);
    return 1;
}

void *malloc(size_t size) {
    void *ret = NULL;

    EFI_STATUS status = BS->AllocatePool(EfiLoaderData, size, &ret);
    if (status != EFI_SUCCESS || !ret) {
        errno = ENOMEM;
        ret = NULL;
    }

    return ret;
}

void *calloc(size_t num, size_t size) {
    void *ret = malloc(num * size);
    if (ret) memset(ret, 0, num * size);
    return ret;
}

void *realloc(void *ptr, size_t size) {
    void *ret = NULL;
 
    if (!ptr) return malloc(size);
    if (!size) {
        free(ptr);
        return NULL;
    }

    EFI_STATUS status = BS->AllocatePool(EfiLoaderData, size, &ret);
    if (status != EFI_SUCCESS || !ret) {
        errno = ENOMEM;
        ret = NULL;
    }

    memcpy(ret, ptr, size);
    free(ptr);

    return ret;
}

void free(void *ptr) {
    if (!ptr) return;
    EFI_STATUS status = BS->FreePool(ptr);
    if (status != EFI_SUCCESS) errno = ENOMEM;
}

int abs(int i) {
    return i < 0 ? -i : i;
}

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

size_t mbstowcs(wchar_t *dest, const char *src, size_t size) {
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

void exit(int status) {
    BS->Exit(IM, !status ? 0 : (status < 0 ? EFIERR(-status) : EFIERR(status)), 0, NULL);
}

void abort(void) {
    BS->Exit(IM, EFI_ABORTED, 0, NULL);
}
