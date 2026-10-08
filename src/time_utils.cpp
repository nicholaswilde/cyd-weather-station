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

#endif // NATIVE_TEST
