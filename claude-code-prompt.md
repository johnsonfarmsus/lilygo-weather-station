# WiFi Triangulation Update Prompt for Claude-code

I need you to update my LilyGO T-Display S3 weather station code to add dynamic location detection using WiFi triangulation. Currently, the weather station uses a hardcoded zip code/location, but I want it to automatically detect its current location when connecting to different WiFi networks (like my phone's hotspot at conventions).

## Current Repository:
https://github.com/johnsonfarmsus/lilygo-weather-station

## ⚠️ IMPORTANT: API Key Security
**This code will be shared publicly on GitHub, so we MUST protect API keys from exposure:**

1. **Never commit API keys to the repository**
2. **Create a `secrets.h.template` file** with placeholder values that users can copy and rename
3. **Add `secrets.h` to `.gitignore`** to prevent accidental commits
4. **Use `#include "secrets.h"` in main code** for actual API key variables
5. **Provide clear documentation** on how users should create their own `secrets.h` file
6. **Add safety checks** to warn if default/empty API keys are being used

## Requirements:

### 1. **Add WiFi Triangulation Using Google Geolocation API:**
   - Integrate the WifiLocation library (https://github.com/gmag11/WifiLocation) 
   - Scan nearby WiFi access points and send BSSID data to Google Geolocation API
   - Return latitude/longitude coordinates for weather API calls

### 2. **API Key Security Implementation:**
   ```cpp
   // Create secrets.h.template with:
   #define OPENWEATHER_API_KEY "your_openweather_api_key_here"
   #define GOOGLE_GEOLOCATION_API_KEY "your_google_geolocation_api_key_here"
   
   // In main code, add validation:
   #ifdef OPENWEATHER_API_KEY
     #if strcmp(OPENWEATHER_API_KEY, "your_openweather_api_key_here") == 0
       #error "Please set your actual API keys in secrets.h"
     #endif
   #else
     #error "secrets.h file missing - copy from secrets.h.template"
   #endif
   ```

### 3. **Dynamic Location Updates:**
   - Check location every 2-4 hours (less frequent than weather updates to save API calls)
   - Compare new coordinates with previous ones
   - Only update weather if location changed significantly (>5km radius)
   - Cache location data to avoid excessive API calls

### 4. **Fallback Mechanism:**
   - Keep existing zip code configuration as backup
   - If WiFi triangulation fails, fall back to hardcoded location
   - Add error handling and timeout for location requests

### 5. **API Integration:**
   - Add Google Geolocation API key configuration (securely)
   - Modify existing OpenWeatherMap API calls to use lat/lon instead of zip code
   - Handle API rate limits appropriately

### 6. **Code Structure Changes:**
   ```cpp
   // Add these functions:
   - getLocationFromWiFi() // Returns location_t with lat/lon
   - shouldUpdateLocation() // Check if location update needed
   - updateWeatherByLocation(float lat, float lon) // Modified weather function
   - locationChanged(location_t newLoc, location_t oldLoc) // Compare locations
   - validateAPIKeys() // Check that API keys are properly set
   ```

### 7. **Secure Configuration Updates:**
   - Create `secrets.h.template` with placeholder API keys
   - Add `secrets.h` to `.gitignore`
   - Update README.md with clear API key setup instructions
   - Add location update interval setting to main config
   - Add location change threshold setting

### 8. **Display Enhancements:**
   - Show current city name on display (if space allows)
   - Add location update indicator
   - Show location accuracy/status
   - Display warning if API keys are not configured

### 9. **Library Dependencies:**
   - Add WifiLocation library to platformio.ini
   - Ensure ArduinoJson is properly configured for geolocation response parsing

### 10. **Documentation & Security:**
   - Update README.md with:
     - Google Geolocation API setup instructions
     - How to get API keys securely
     - How to create and configure secrets.h
     - API usage limits and costs
     - Security best practices

## Technical Specifications:
- ESP32-S3 with ST7789 170x320 display
- Currently using OpenWeatherMap API with 30-minute weather updates
- PlatformIO project structure
- Should maintain existing weather display functionality

## Expected Behavior:
1. **On startup**: Validate API keys → Get location via WiFi triangulation → Get weather
2. **During operation**: Check location every 2-4 hours
3. **If location changed significantly**: Update weather for new location
4. **If geolocation fails**: Use fallback zip code method
5. **Display**: Show current location name and update status when possible
6. **Error handling**: Clear user feedback for API key issues, network problems, etc.

## File Structure After Update:
```
├── src/
├── include/
├── platformio.ini
├── secrets.h.template    # Template with placeholder keys
├── .gitignore           # Must include secrets.h
├── README.md            # Updated with setup instructions
└── secrets.h            # User creates this (git-ignored)
```

## Security Checklist:
- [ ] `secrets.h` added to `.gitignore`
- [ ] `secrets.h.template` created with placeholders
- [ ] API key validation added to prevent default values
- [ ] Clear error messages for missing/invalid API keys
- [ ] README updated with secure setup instructions
- [ ] No hardcoded API keys anywhere in committed code

Please update all necessary files and provide clear, secure instructions for obtaining and configuring API keys. Maintain the existing clean UI and ensure the location detection doesn't significantly impact the 30-minute weather update cycle.

Focus on reliability, security, and battery efficiency since this will be used as a portable convention name tag shared publicly on GitHub.