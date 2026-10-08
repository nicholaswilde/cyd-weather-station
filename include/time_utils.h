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

#endif // TIME_UTILS_H
