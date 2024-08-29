#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

void *memchr(const void *s, int c, size_t size) {
    uint8_t *p = (uint8_t *)s;
    uint8_t cc = (uint8_t)c;

    for (size_t i = 0; i < size; i++, p++) {
        if (*p == cc)
            return p;
    }

    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t size) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < size; i++) {
        if (p1[i] != p2[i])
            return p1[i] < p2[i] ? -1 : 1;
    }

    return 0;
}

void *memcpy(void *dest, const void *src, size_t size) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < size; i++)
        pdest[i] = psrc[i];

    return dest;
}

void *memmove(void *dest, const void *src, size_t size) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (dest < src) {
        for (size_t i = 0; i < size; i++)
            pdest[i] = psrc[i];
    } else {
        for (size_t i = size; i != 0; i--)
            pdest[i - 1] = psrc[i - 1];
    }

    return dest;
}

void *memset(void *buffer, int value, size_t size) {
    uint8_t *pbuffer = (uint8_t *)buffer;

    for (size_t i = 0; i < size; i++)
        pbuffer[i] = (uint8_t)value;

    return buffer;
}

char *strchr(const char *str, int ch) {
    while (*str != (char)ch)
        if (!*str++)
            return 0;
    return (char *)str;
}

char *strcpy(char *dest, const char *src) {
    size_t i;

    for (i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i]; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = 0;

    return dest;
}

char *strcat(char *dest, const char *src) {
    char *s = dest;

    if (src && dest) {
        dest += strlen(dest);
        while (*src) {
            *dest++ = *src++;
        }
        *dest = 0;
    }

    return s;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *s = dest;
    const char *e = src + n;

    if (src && dest && n > 0) {
        dest += strlen(dest);
        while (*src && src < e) {
            *dest++ = *src++;
        }
        *dest = 0;
    }

    return s;
}

size_t strlen(const char *str) {
    size_t len = 0;

    while (str[len] != '\0')
        len++;

    return len;
}

char *strpbrk(const char *s, const char *chrs) {
    size_t n = 0;
    while (s[n]) {
        if (strchr(chrs, s[n]))
            return (char*)(s + n);
        n++;
    }
    return NULL;
}

int strcmp(const char *s1, const char *s2) {
    for (size_t i = 0; ; i++) {
        char c1 = s1[i], c2 = s2[i];
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            return 0;
    }
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        char c1 = s1[i], c2 = s2[i];
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            return 0;
    }

    return 0;
}

size_t strspn(const char *s1, const char *s2) {
    size_t ret = 0;

    while (*s1 && strchr(s2, *s1++))
        ret++;

    return ret;    
}

size_t strcspn(const char *s1, const char *s2) {
    size_t ret = 0;

    while (*s1) {
        if (strchr(s2, *s1))
            return ret;
        else
            s1++, ret++;
    }

    return ret;
}

char *strstr(const char *str, const char *pattern) {
    for (size_t i = 0; str[i]; i++) {
        bool found = true;
        for (size_t j = 0; pattern[j]; j++) {
            if (!pattern[j] || str[i + j] == pattern[j]) continue;

            found = false;
            break;
        }

        if (found) return (char*)(&str[i]);
    }

    return NULL;
}

char *strtok(char *str, const char *delim) {
    char *p = 0;

    if (str)
        p = str;
    else if (!p)
        return 0;

    str = p + strspn(p, delim);
    p = str + strcspn(str, delim);
    if (p == str)
        return p = 0;

    p = *p ? *p = 0,p + 1 : 0;
    return str;
}
