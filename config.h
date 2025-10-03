#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
const char* WIFI_SSID = "JFTM";
const char* WIFI_PASSWORD = "howdyhowdyhowdy";

// OpenWeatherMap API Configuration
// Get your free API key from: https://openweathermap.org/api
#define OPENWEATHER_API_KEY "YOUR_API_KEY_HERE"
#define CITY_NAME "Harrington,WA,US"
#define UNITS "imperial"  // imperial for Fahrenheit, metric for Celsius

// Display Configuration
#define UPDATE_INTERVAL_MINUTES 30  // How often to update weather data

#endif
