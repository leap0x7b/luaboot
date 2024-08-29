#include <ctype.h>

// yes i know i only need stuff like isalnum, isalpha, isascii, isdigit, islower, isupper, tolower, toupper, etc but fuck it

int isalnum(char c) {
    return isalpha(c) || isdigit(c);
}

int isalpha(char c) {
    return (c | 32) - 'a' < 26;
}

int isascii(char c) {
    return !(c & ~0x7f);
}

int isblank(char c) {
    return (c == ' ' || c == '\t');
}

int iscntrl(char c) {
    return c < 0x20 || c == 0x7f;
}

int isdigit(char c) {
    return c - '0' < 10;
}

int isgraph(char c) {
    return c - 0x21 < 0x5e;
}

int islower(char c) {
    return c - 'a' < 26;
}

int isprint(char c) {
    return c - 0x20 < 0x5f;
}

int ispunct(char c) {
    return isgraph(c) && !isalnum(c);
}

int isspace(char c) {
    return c == ' ' || c - '\t' < 5;
}

int isupper(char c) {
    return c - 'A' < 26;
}

int isxdigit(char c) {
    return isdigit(c) || (c | 32) - 'a' < 6;
}

char tolower(char c) {
    if (isupper(c)) return c | 32;
    return c;
}

char toupper(char c) {
    if (islower(c)) return c & 0x5f;
    return c;
}
