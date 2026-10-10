#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stddef.h>
#include <Arduino.h>

/**
 * Populates info with local time based on the user's configured timezone.
 * Returns true if valid local time could be obtained, false otherwise.
 */
bool getLocalTimeWrapper(struct tm* info, uint32_t ms = 5000);

/**
 * Populates today_str and tomorrow_str formatted as "YYYY-MM-DD"
 * in the user's configured timezone.
 *
 * @param today_str Output buffer for today's date string.
 * @param today_size Size of today_str buffer.
 * @param tomorrow_str Output buffer for tomorrow's date string.
 * @param tomorrow_size Size of tomorrow_str buffer.
 * @param now_override Optional timestamp override (0 for current time).
 * @return true if valid local date strings could be computed (NTP synced), false otherwise.
 */
bool getLocalDateStrings(char* today_str, size_t today_size,
                         char* tomorrow_str, size_t tomorrow_size,
                         time_t now_override = 0);

/**
 * Converts a UTC timestamp into a local calendar date string "YYYY-MM-DD"
 * and optionally retrieves the local hour (0-23) based on the configured timezone.
 *
 * @param utc_ts Unix epoch timestamp in UTC seconds.
 * @param date_str Output buffer for date string (at least 11 bytes).
 * @param date_size Size of date_str buffer.
 * @param local_hour Optional pointer to int to receive the local hour (0-23).
 * @return true if successfully converted, false otherwise.
 */
bool getLocalDateStringFromTimestamp(time_t utc_ts, char* date_str, size_t date_size,
                                     int* local_hour = nullptr);

/**
 * Parses an ISO UTC date-time string like "YYYY-MM-DD HH:MM:SS" into UTC epoch seconds.
 *
 * @param dt_txt String formatted as "YYYY-MM-DD HH:MM:SS".
 * @return UTC epoch seconds, or 0 on parse failure.
 */
time_t parseUtcDtTxt(const char* dt_txt);

#endif // TIME_UTILS_H
