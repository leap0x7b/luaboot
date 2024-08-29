#include <stdlib.h>
#include <time.h>
#include <luaboot/e9.h>
#include <luaboot/efi.h>

static struct tm tm;

/* from musl */
static uint64_t year_to_secs(uint64_t year, int *is_leap) {
    uint64_t y, cycles, centuries, leaps, rem;

    if (year - 2ULL <= 136) {
        y = (int)year;
        leaps = (y - 68) >> 2;

        if (!((y - 68) & 3)) {
            leaps--;
            if (is_leap)
                *is_leap = 1;
        } else if (is_leap)
            *is_leap = 0;

        return 31536000ULL * (y - 70) + 86400ULL * leaps;
    }

    if (!is_leap)
        *is_leap = 0;

    cycles = (year - 100) / 400;
    rem = (year - 100) % 400;

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
            if (rem >= 300) {
                centuries = 3;
                rem -= 300;
            } else {
                centuries = 2;
                rem -= 200;
            }
        } else {
            if (rem >= 100) {
                centuries = 1;
                rem -= 100;
            } else
                centuries = 0;
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

    leaps += 97 * cycles + 24 * centuries - *is_leap;

    return (year - 100) * 31536000ULL + leaps * 86400ULL + 946684800ULL + 86400ULL;
}

static time_t mktime_efi(EFI_TIME *t) {
    tm.tm_year = t->Year + (/* workaround some buggy firmware*/ t->Year > 2000 ? -1900 : 98);
    tm.tm_mon = t->Month - 1;
    tm.tm_mday = t->Day;
    tm.tm_hour = t->Hour;
    tm.tm_min = t->Minute;
    tm.tm_sec = t->Second;
    tm.tm_isdst = t->Daylight;

    return mktime(&tm);
}

struct tm *localtime (const time_t *) {
    EFI_TIME t;
    RT->GetTime(&t, NULL);

    mktime_efi(&t);
    return &tm;
}

time_t mktime(const struct tm *tm) {
    time_t ret;
    static const uint64_t secs_through_month[] = {
        0,           // January
        31 * 86400,  // February
        59 * 86400,  // March
        90 * 86400,  // April
        120 * 86400, // May
        151 * 86400, // June
        181 * 86400, // July
        212 * 86400, // August
        243 * 86400, // September
        273 * 86400, // October
        304 * 86400, // November
        334 * 86400  // December
    };

    int is_leap;
    uint64_t year = tm->tm_year;
    uint64_t adj;
    uint64_t month = tm->tm_mon;

    if (month >= 12 || month < 0) {
        adj = month / 12;
        month %= 12;
        if (month < 0) {
            adj--;
            month += 12;
        }
        year += adj;
    }

    ret = year_to_secs(year, &is_leap);
    ret += secs_through_month[month];
    if (is_leap && month >= 2)
        ret += 86400;
    ret += 86400ULL * (tm->tm_mday - 1);
    ret += 3600ULL * tm->tm_hour;
    ret += 60ULL * tm->tm_min;
    ret += tm->tm_sec;

    return ret;
}

time_t time(time_t *time) {
    time_t ret;
    EFI_TIME t;

    RT->GetTime(&t, NULL);
    ret = mktime_efi(&t);

    if (time)
        *time = ret;

    return ret;
}

struct tm *gmtime(const time_t *time) {
    return localtime(time);
}