# Specification: sd-weather-logging_20260711

## Overview
Implement periodic logging of key weather and system variables (temperature, humidity, wind speed, wind direction, weather code) to a CSV format file (`weather_history.csv`) stored on the SD card.

## Functional Requirements
1. **SD Card Initialization**: Initialize the SD card interface via SPI. Handle missing card insertion gracefully without crashing.
2. **Periodic Logs**: Every hour (aligned with the weather fetch process), append a new comma-separated values (CSV) row to the file.
3. **Data Schema**: The CSV format must include: `Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode`.
4. **Error Handling**: Fail silently if the SD card is removed during execution, and attempt re-initialization on the next write attempt.

## Acceptance Criteria
- Inserted SD cards are successfully mounted on boot.
- A file named `weather_history.csv` is created on the SD root directory if it does not exist.
- Each weather update writes a new line containing valid timestamped parameters to the log file.
