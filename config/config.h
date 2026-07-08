#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Open-Meteo API Settings
#define WEATHER_API_LATITUDE "37.7749"
#define WEATHER_API_LONGITUDE "-122.4194"
#define WEATHER_UPDATE_INTERVAL_MINS 15

// NTP and Timezone Settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC (-8 * 3600)  // GMT -8 (PST)
#define DST_OFFSET_SEC 3600         // Daylight Savings offset (1 hour)

#endif // CONFIG_H
