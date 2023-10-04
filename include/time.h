#pragma once

#include <stdint.h>
#include <string.h>

#define CLOCKS_PER_SEC 1000000

typedef int64_t time_t;
typedef int64_t clock_t;

struct tm {
    uint64_t tm_year;
    uint64_t tm_mon;
    uint64_t tm_mday;
    uint64_t tm_hour;
    uint64_t tm_min;
    uint64_t tm_sec;
    uint64_t tm_yday;
    uint64_t tm_wday;
    uint64_t tm_isdst;
};

struct tm* gmtime(const time_t*);
time_t time(time_t*);
struct tm* localtime(const time_t*);
static inline clock_t clock() {
    return /* we spent approximately no time at all */ 0;
}
static inline uint64_t strftime(char *s, uint64_t max,
    const char *format,
    const struct tm *tm) {
    (void)max; (void)format; (void)tm;
    strcpy(s, "<idfk time>");
    return 11;
}
time_t mktime(const struct tm* tm);
static inline time_t difftime(time_t l, time_t r) { return l - r; }