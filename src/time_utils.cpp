#include "time_utils.h"
#include "settings_manager.h"
#include <stdio.h>
#include <stdlib.h>

extern SettingsManager settings;

#ifndef NATIVE_TEST
#include <AceTime.h>
using namespace ace_time;

extern bool ntpInitialized;

static const int CACHE_SIZE = 1;
static BasicZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static BasicZoneManager zoneManager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry,
    zoneProcessorCache);

bool getLocalTimeWrapper(struct tm* info, uint32_t ms) {
    if (!info) return false;
    if (!ntpInitialized) return false;
    time_t now = time(nullptr);
    if (now < 946684800L) return false;
    TimeZone tz = zoneManager.createForZoneName(settings.getTimezone().c_str());
    if (tz.isError()) tz = zoneManager.createForZoneName("UTC");
    ZonedDateTime zdt = ZonedDateTime::forUnixSeconds64(now, tz);
    if (zdt.isError()) return false;
    info->tm_year = zdt.year() - 1900;
    info->tm_mon = zdt.month() - 1;
    info->tm_mday = zdt.day();
    info->tm_hour = zdt.hour();
    info->tm_min = zdt.minute();
    info->tm_sec = zdt.second();
    info->tm_wday = (zdt.dayOfWeek() == 7) ? 0 : zdt.dayOfWeek();
    return true;
}

bool getLocalDateStrings(char* today_str, size_t today_size,
                         char* tomorrow_str, size_t tomorrow_size,
                         time_t now_override) {
    time_t now = now_override;
    if (now == 0) {
        if (!ntpInitialized) return false;
        now = time(nullptr);
    }
    if (now < 946684800L) {
        return false;
    }

    TimeZone tz = zoneManager.createForZoneName(settings.getTimezone().c_str());
    if (tz.isError()) tz = zoneManager.createForZoneName("UTC");
    ZonedDateTime zdt = ZonedDateTime::forUnixSeconds64(now, tz);
    if (zdt.isError()) return false;

    LocalDate local_today = LocalDate::forComponents(zdt.year(), zdt.month(), zdt.day());
    LocalDate local_tomorrow = LocalDate::forEpochDays(local_today.toEpochDays() + 1);

    if (today_str && today_size > 0) {
        snprintf(today_str, today_size, "%04d-%02d-%02d",
                 local_today.year(), local_today.month(), local_today.day());
    }

    if (tomorrow_str && tomorrow_size > 0) {
        snprintf(tomorrow_str, tomorrow_size, "%04d-%02d-%02d",
                 local_tomorrow.year(), local_tomorrow.month(), local_tomorrow.day());
    }

    return true;
}

bool getLocalDateStringFromTimestamp(time_t utc_ts, char* date_str, size_t date_size,
                                     int* local_hour) {
    if (!date_str || date_size == 0) return false;
    if (utc_ts == 0) return false;

    TimeZone tz = zoneManager.createForZoneName(settings.getTimezone().c_str());
    if (tz.isError()) tz = zoneManager.createForZoneName("UTC");
    ZonedDateTime zdt = ZonedDateTime::forUnixSeconds64(utc_ts, tz);
    if (zdt.isError()) return false;

    snprintf(date_str, date_size, "%04d-%02d-%02d",
             zdt.year(), zdt.month(), zdt.day());
    if (local_hour) {
        *local_hour = zdt.hour();
    }
    return true;
}

#else // NATIVE_TEST

bool ntpInitialized = true;

bool getLocalTimeWrapper(struct tm* info, uint32_t ms) {
    if (!info) return false;
    time_t now = time(nullptr);
    if (now < 946684800L) return false;

    setenv("TZ", settings.getTimezone().c_str(), 1);
    tzset();

    localtime_r(&now, info);
    return true;
}

bool getLocalDateStrings(char* today_str, size_t today_size,
                         char* tomorrow_str, size_t tomorrow_size,
                         time_t now_override) {
    time_t now = now_override;
    if (now < 946684800L) {
        return false;
    }

    setenv("TZ", settings.getTimezone().c_str(), 1);
    tzset();

    struct tm tm_today;
    localtime_r(&now, &tm_today);

    if (today_str && today_size > 0) {
        snprintf(today_str, today_size, "%04d-%02d-%02d",
                 tm_today.tm_year + 1900, tm_today.tm_mon + 1, tm_today.tm_mday);
    }

    if (tomorrow_str && tomorrow_size > 0) {
        struct tm tm_tmrw = tm_today;
        tm_tmrw.tm_mday += 1;
        tm_tmrw.tm_isdst = -1;
        mktime(&tm_tmrw);
        snprintf(tomorrow_str, tomorrow_size, "%04d-%02d-%02d",
                 tm_tmrw.tm_year + 1900, tm_tmrw.tm_mon + 1, tm_tmrw.tm_mday);
    }

    return true;
}

bool getLocalDateStringFromTimestamp(time_t utc_ts, char* date_str, size_t date_size,
                                     int* local_hour) {
    if (!date_str || date_size == 0) return false;
    if (utc_ts == 0) return false;

    setenv("TZ", settings.getTimezone().c_str(), 1);
    tzset();

    struct tm tm_info;
    localtime_r(&utc_ts, &tm_info);
    snprintf(date_str, date_size, "%04d-%02d-%02d",
             tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday);
    if (local_hour) {
        *local_hour = tm_info.tm_hour;
    }
    return true;
}

#endif // NATIVE_TEST

time_t parseUtcDtTxt(const char* dt_txt) {
    if (!dt_txt || strlen(dt_txt) < 19) return 0;
    int y = (dt_txt[0]-'0')*1000 + (dt_txt[1]-'0')*100 + (dt_txt[2]-'0')*10 + (dt_txt[3]-'0');
    int m = (dt_txt[5]-'0')*10 + (dt_txt[6]-'0');
    int d = (dt_txt[8]-'0')*10 + (dt_txt[9]-'0');
    int hh = (dt_txt[11]-'0')*10 + (dt_txt[12]-'0');
    int mm = (dt_txt[14]-'0')*10 + (dt_txt[15]-'0');
    int ss = (dt_txt[17]-'0')*10 + (dt_txt[18]-'0');

#ifndef NATIVE_TEST
    TimeZone tz = zoneManager.createForZoneName("UTC");
    ZonedDateTime zdt = ZonedDateTime::forComponents(y, m, d, hh, mm, ss, tz);
    if (zdt.isError()) return 0;
    return (time_t)zdt.toUnixSeconds64();
#else
    struct tm tm_utc = {};
    tm_utc.tm_year = y - 1900;
    tm_utc.tm_mon = m - 1;
    tm_utc.tm_mday = d;
    tm_utc.tm_hour = hh;
    tm_utc.tm_min = mm;
    tm_utc.tm_sec = ss;
    return timegm(&tm_utc);
#endif
}

