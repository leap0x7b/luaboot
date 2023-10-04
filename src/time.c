#include <time.h>
#include <luaboot/e9.h>
#include <luaboot/efi.h>
#include <luaboot/stdlib.h>

static struct tm __tm;
time_t __mktime_efi(EFI_TIME *t);

/* from musl */
static uint64_t __year_to_secs(uint64_t year, int *is_leap)
{
    int y, cycles, centuries, leaps, rem;

    if (year-2ULL <= 136) {
        y = (int)year;
        leaps = (y-68)>>2;
        if (!((y-68)&3)) {
            leaps--;
            if (is_leap) *is_leap = 1;
        } else if (is_leap) *is_leap = 0;
        return 31536000ULL*(uint64_t)(y-70) + 86400ULL*(uint64_t)leaps;
    }

    if (!is_leap) is_leap = &(int){0};
    cycles = (int)((year-100) / 400);
    rem = (year-100) % 400;
    if (rem < 0) {
        cycles--;
        rem += 400;
    }
    if (!rem) {
        *is_leap = 1;
        centuries = 0;
        leaps = 0;
    } else {
        if (rem >= 200) {
            if (rem >= 300) { centuries = 3; rem -= 300; }
            else { centuries = 2; rem -= 200; }
        } else {
            if (rem >= 100) { centuries = 1; rem -= 100; }
            else centuries = 0;
        }
        if (!rem) {
            *is_leap = 0;
            leaps = 0;
        } else {
            leaps = rem / 4;
            rem %= 4;
            *is_leap = !rem;
        }
    }

    leaps += 97*cycles + 24*centuries - *is_leap;

    return (uint64_t)(year-100) * 31536000ULL + (uint64_t)leaps * 86400ULL + 946684800ULL + 86400ULL;
}

time_t __mktime_efi(EFI_TIME *t)
{
    __tm.tm_year = t->Year + (/* workaround some buggy firmware*/ t->Year > 2000 ? -1900 : 98);
    __tm.tm_mon = t->Month - 1;
    __tm.tm_mday = t->Day;
    __tm.tm_hour = t->Hour;
    __tm.tm_min = t->Minute;
    __tm.tm_sec = t->Second;
    __tm.tm_isdst = t->Daylight;
    return mktime(&__tm);
}

/**
 * This isn't POSIX, no arguments. Just returns the current time in struct tm
 */
struct tm *localtime (const time_t *__timer)
{
    EFI_TIME t = {0};
    (void)__timer;
    ST->RuntimeServices->GetTime(&t, NULL);
    __mktime_efi(&t);
    return &__tm;
}

time_t mktime(const struct tm *tm)
{
    static const uint64_t secs_through_month[] = {
        0, 31*86400, 59*86400, 90*86400,
        120*86400, 151*86400, 181*86400, 212*86400,
        243*86400, 273*86400, 304*86400, 334*86400 };
    int is_leap;
    uint64_t year = (uint64_t)tm->tm_year, t, adj;
    int month = tm->tm_mon;
    if (month >= 12 || month < 0) {
        adj = (uint64_t)month / 12;
        month %= 12;
        if (month < 0) {
            adj--;
            month += 12;
        }
        year += adj;
    }
    t = __year_to_secs(year, &is_leap);
    t += secs_through_month[month];
    if (is_leap && month >= 2) t += 86400;
    t += 86400ULL * (uint64_t)(tm->tm_mday-1);
    t += 3600ULL * (uint64_t)tm->tm_hour;
    t += 60ULL * (uint64_t)tm->tm_min;
    t += (uint64_t)tm->tm_sec;
    return (time_t)t;
}

time_t time(time_t *__timer)
{
    time_t ret;
    EFI_TIME t = {0};
    ST->RuntimeServices->GetTime(&t, NULL);
    ret = __mktime_efi(&t);
    if(__timer) *__timer = ret;
    return ret;
}

struct tm *gmtime(const time_t *time) {
    return localtime(time);
}