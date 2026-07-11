# Specification: multi-city-carousel_20260711

## Overview
Implement a location carousel that cycles weather data between multiple cities. Users can configure a list of ZIP codes or coordinates, and the device will rotate current weather conditions periodically (e.g. every 30 seconds) or when the user double-taps the weather icon.

## Functional Requirements
1. **Multi-Location Storage**: Store an array of locations in settings.
2. **Carousel Rotation**: Cycle through the active queue of cities.
3. **Double-Tap / Swipe Action**: Detect double-taps on the main weather icon to manually skip to the next city.
4. **Smooth Transitions**: Animate updates on screen when switching cities.

## Acceptance Criteria
- Weather updates are fetched for all configured locations.
- The screen cycles through configured cities automatically at the set interval.
- Double-tapping manually triggers an instantaneous switch to the next city in queue.
