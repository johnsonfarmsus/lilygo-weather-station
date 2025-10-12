# LilyGo T-Display S3 Weather Station

A beautiful, real-time weather display showing current conditions and 3-day forecast for any location using the LilyGo T-Display S3 and OpenWeatherMap API. Now with **WiFi triangulation** for automatic location detection!

![T-Display S3 Weather Station](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![Version](https://img.shields.io/badge/Version-1.3.0-green)
![License](https://img.shields.io/badge/License-MIT-blue)

## Features

- 🌡️ **Current Temperature** - Large, easy-to-read display in cyan
- 📊 **3-Day Forecast** - Today's high/low plus next 2 days
- 🌤️ **Weather Conditions** - Real-time weather descriptions
- 🔄 **Auto-Update** - Refreshes every 30 minutes
- ⚠️ **Stale Data Warning** - Red asterisk if data is >2 hours old
- 🎨 **Clean UI** - Horizontal tile layout with optimized fonts
- 📱 **WiFi Connected** - Automatic connection on startup
- 🌍 **Automatic Timezone** - Detects timezone from location coordinates
- 📅 **Smart Day Names** - Accurate day-of-week that updates properly
- 📍 **NEW: WiFi Triangulation** - Automatically detects your location (no GPS needed!)
- 🔒 **Secure API Management** - API keys stored safely, never committed to Git

## Display Layout

```
┌─────────────────────────────────────┐
│  62°F      Fri       Sat            │  ← Current temp (cyan) | Day/Date
│           10-04     10-05            │
│                                      │
│ 72/58°F  68/52°F   61/48°F          │  ← High/Low temps (yellow)
│                                      │
│ Clear sky Overcast  Scattered       │  ← Conditions
│          clouds     clouds           │
└─────────────────────────────────────┘
```

## Hardware Requirements

- **LilyGo T-Display S3** (ESP32-S3 with 170x320 ST7789 display)
- USB-C cable for programming
- WiFi network access

## Software Requirements

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- [OpenWeatherMap API key](https://openweathermap.org/api) (free tier works great)
- [Google Geolocation API key](https://console.cloud.google.com/) (free tier: 40,000 requests/month)

## 🔐 Important: API Keys & Security

**This project uses a secure configuration system to protect your API keys:**

- API keys and WiFi credentials are stored in a **`secrets.h`** file
- `secrets.h` is **git-ignored** and will never be committed to version control
- You create `secrets.h` from the provided template
- The code will not compile without properly configured API keys

**Never commit `secrets.h` to your repository!**

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/johnsonfarmsus/lilygo-weather-station.git
cd lilygo-weather-station
```

### 2. Get Your API Keys

#### OpenWeatherMap API Key (Required)
1. Sign up at [OpenWeatherMap](https://home.openweathermap.org/users/sign_up)
2. Navigate to **API Keys** in your account
3. Copy your API key (or generate a new one)
4. **Free tier**: 1,000 calls/day (plenty for this project!)

#### Google Geolocation API Key (Required for WiFi Triangulation)
1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Create a new project (or select existing)
3. Enable the **Geolocation API**:
   - Go to "APIs & Services" → "Library"
   - Search for "Geolocation API"
   - Click "Enable"
4. Create credentials:
   - Go to "APIs & Services" → "Credentials"
   - Click "Create Credentials" → "API Key"
   - Copy your API key
5. **Free tier**: 40,000 requests/month (checking every 2 hours = ~360/month)

### 3. Configure Your Secrets

**Create your secrets file:**

```bash
# Copy the template
cp secrets.h.template secrets.h
```

**Edit `secrets.h` and fill in your values:**

```cpp
// WiFi Settings
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"

// API Keys
#define OPENWEATHER_API_KEY "your_actual_openweather_key_here"
#define GOOGLE_GEOLOCATION_API_KEY "your_actual_google_key_here"

// Fallback Location (Optional but recommended)
// Used if WiFi triangulation fails
#define FALLBACK_LATITUDE "47.6062"      // Example: Seattle, WA
#define FALLBACK_LONGITUDE "-122.3321"
#define USE_FALLBACK_LOCATION false      // Set to true to always use fallback
```

**To find coordinates for fallback location:**
- Go to [Google Maps](https://maps.google.com)
- Right-click your location
- Copy the latitude and longitude values
- Or use: https://www.latlong.net/

### 4. Upload to Device

**Using PlatformIO (Recommended):**

```bash
# Install PlatformIO if you haven't already
pip install platformio

# Connect your T-Display S3 via USB-C
# Compile and upload
pio run --target upload

# Optional: Monitor serial output to see location detection
pio device monitor --baud 115200
```

**Using Arduino IDE:**

1. Install ESP32 board support
2. Select board: **ESP32S3 Dev Module** or **LilyGo T-Display-S3**
3. Copy all files to Arduino sketch folder
4. Install required libraries:
   - LVGL 8.3+
   - ArduinoJson 7.0+
   - WifiLocation 1.1.0+
5. Upload to board

## How WiFi Triangulation Works

**No GPS needed!** The device automatically detects your location by:

1. **Scanning nearby WiFi networks** - Looks for visible WiFi access points
2. **Sending data to Google** - Google's Geolocation API identifies your location
3. **Updating weather** - Fetches weather for your current location
4. **Periodic checks** - Checks location every 2 hours
5. **Movement detection** - Updates weather only if you've moved >5km

**Perfect for:**
- Convention name tags (moves with you!)
- Travel displays
- Portable weather stations
- Multi-location use

**Fallback protection:**
- If triangulation fails, uses your configured fallback location
- Displays clear error messages if APIs aren't configured

## Configuration Options

### Update Frequency

Edit `main.cpp` to change refresh intervals:

```cpp
// Weather update interval (default: 30 minutes)
#define UPDATE_INTERVAL_MS (30 * 60 * 1000)

// Location check interval (default: 2 hours)
const unsigned long LOCATION_UPDATE_INTERVAL = (2 * 60 * 60 * 1000);

// Movement threshold (default: 5km)
const float LOCATION_CHANGE_THRESHOLD_KM = 5.0;
```

### Temperature Units

Switch between Fahrenheit and Celsius in `main.cpp`:

```cpp
#define UNITS "imperial"  // Fahrenheit
// or
#define UNITS "metric"    // Celsius
```

### Location Mode

In `secrets.h`, you can control location behavior:

```cpp
// Always use fallback location (disables WiFi triangulation)
#define USE_FALLBACK_LOCATION false  // Set to true for fixed location

// Fallback coordinates (used if triangulation fails or if forced)
#define FALLBACK_LATITUDE "47.6062"
#define FALLBACK_LONGITUDE "-122.3321"
```

**When to use fallback mode:**
- Testing without moving
- Fixed installation (no need for triangulation)
- Areas with poor WiFi coverage

## Display Details

### Color Scheme
- **Cyan (0x00FFFF)**: Current temperature and conditions
- **Yellow (0xFFFF00)**: High/Low temperatures (fresh data)
- **Red (0xFF0000)**: Temperatures when data is stale (>2 hours)
- **White (0xFFFFFF)**: Day/Date labels
- **Gray (0xAAAAAA)**: Future forecast descriptions

### Font Sizes
- Current temperature: 26pt
- High/Low temperatures: 24pt
- Weather descriptions: 18pt
- Day/Date labels: 12pt

## Troubleshooting

### Compilation Errors

**Error: "secrets.h file missing"**
- Copy `secrets.h.template` to `secrets.h`
- Fill in your actual API keys and WiFi credentials

**Error: "API key not configured"**
- You're still using placeholder values in `secrets.h`
- Replace `"your_openweather_api_key_here"` with your actual API key
- Replace `"your_google_geolocation_api_key_here"` with your actual API key

### Location Detection Issues

**"Location detection failed"**
- Check that your Google Geolocation API key is correct
- Ensure the Geolocation API is enabled in Google Cloud Console
- Verify you have nearby WiFi networks visible (needs at least 2-3 networks)
- Check serial monitor for detailed error messages
- If persistent, configure and use fallback location

**Location not updating when moving**
- Location checks every 2 hours by default
- Must move >5km to trigger update
- Check serial monitor during location checks
- Verify WiFi is still connected

### Display Not Working
- Ensure GPIO15 is powered (handled automatically in code)
- The T-Display S3 uses **parallel I80 interface**, not SPI
- Check that LVGL is properly configured

### WiFi Connection Issues
- Verify SSID and password in `secrets.h` (not `pin_config.h`!)
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Serial monitor will show connection status

### API Errors

**OpenWeatherMap:**
- Verify API key is correct and activated (can take 10-15 minutes after creation)
- Check you haven't exceeded free tier limits (1,000 calls/day)
- Free tier is sufficient for this project

**Google Geolocation:**
- Verify API key is correct
- Check Geolocation API is enabled in Cloud Console
- Free tier: 40,000 requests/month (way more than needed)
- This project uses ~360 requests/month at default settings

### Upload Fails
- Try putting device in **programming mode**:
  1. Hold **BOOT** button
  2. Press and release **RESET** button
  3. Release **BOOT** button
  4. Try upload again

### "Stale Data" Warning
- Red asterisk (*) appears on temps if data hasn't updated in 2+ hours
- Usually indicates WiFi connection lost or API issues
- Device will auto-retry on next update cycle

### Serial Monitor Tips

Monitor the serial output at 115200 baud to see:
- API key validation
- WiFi connection status
- Location detection results
- Weather fetch progress
- Error messages with details

## Technical Details

### Libraries Used
- **LVGL 8.3+** - Graphics library for display
- **ArduinoJson 7.0+** - JSON parsing for API responses
- **WiFi** - ESP32 WiFi connection
- **HTTPClient** - API requests
- **WifiLocation 1.1.0+** - WiFi triangulation for location detection

### Display Specs
- **Controller**: ST7789V
- **Resolution**: 320x170 pixels
- **Interface**: 8-bit parallel I80
- **Backlight**: GPIO38
- **Power**: GPIO15

### API Endpoints
Uses two OpenWeatherMap APIs for comprehensive data:
1. **Current Weather API** - Real-time conditions
   ```
   http://api.openweathermap.org/data/2.5/weather
   ```
2. **5-Day Forecast API** - 3-hour interval forecasts
   ```
   http://api.openweathermap.org/data/2.5/forecast
   ```

Both use latitude/longitude for precise location targeting.

### Memory Usage
- **RAM**: ~19KB (6% of 327KB)
- **Flash**: ~1.1MB (17% of 6.5MB)

## Project Structure

```
lilygo-weather-station/
├── src/
│   ├── main.cpp              # Main application code
│   ├── pin_config.h          # Pin definitions
│   └── zones.h               # Timezone data
├── lib/
│   └── lv_conf.h             # LVGL configuration
├── platformio.ini            # PlatformIO configuration
├── secrets.h.template        # Template for API keys (copy to secrets.h)
├── secrets.h                 # Your actual API keys (git-ignored)
├── .gitignore                # Includes secrets.h
└── README.md                 # This file
```

## Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

## License

This project is open source and available under the MIT License.

## Credits

- Built for the [LilyGo T-Display S3](https://www.lilygo.cc/products/t-display-s3)
- Weather data from [OpenWeatherMap](https://openweathermap.org/)
- Uses [LVGL](https://lvgl.io/) graphics library

## Changelog

### v1.3.0 (Current) - WiFi Triangulation & Security Update
- **🎉 NEW**: WiFi triangulation for automatic location detection (no GPS needed!)
- **🔒 SECURITY**: Complete API key security overhaul
  - API keys moved to `secrets.h` (git-ignored)
  - `secrets.h.template` provided for easy setup
  - Compile-time validation prevents use of placeholder keys
  - Clear error messages for configuration issues
- **Added**: Google Geolocation API integration
- **Added**: Periodic location checking (every 2 hours)
- **Added**: Smart weather updates (only when location changes >5km)
- **Added**: User-configurable fallback location
- **Added**: Location distance calculation (Haversine formula)
- **Added**: Enhanced serial output with structured logging
- **Improved**: Better error handling and user feedback
- **Changed**: WiFi credentials moved from `pin_config.h` to `secrets.h`
- **Library**: Added WifiLocation 1.1.0+ dependency

### v1.2.0
- **Added**: Automatic timezone detection from API (works worldwide!)
- **Fixed**: Day-of-week now calculated from actual dates (updates properly)
- **Fixed**: All forecast times converted to local timezone
- **Fixed**: Today's high/low now uses actual forecast data
- **Improved**: NTP syncs to UTC, timezone offset applied from API
- **Note**: No more hardcoded PST - adapts to any location globally

### v1.1.0
- **Fixed**: Coordinate-based location for precise weather data
- **Fixed**: Proper calendar-day grouping for forecasts
- **Fixed**: Current temperature now uses forecast API (more reliable for small towns)
- **Improved**: Better handling of late-day scenarios (no forecast data remaining)
- **Added**: Location verification in serial output

### v1.0.0
- Initial release
- Current temperature and conditions display
- 3-day forecast with high/low temps
- Auto-refresh every 30 minutes
- Stale data warning indicator
- Optimized horizontal tile layout

## Future Enhancements

Potential features for future versions:
- Weather icons/symbols
- Hourly forecast view
- City name display on screen
- Touch screen controls
- Battery level indicator
- Dark/light mode toggle
- Location accuracy indicator

---

**Need help?** Open an issue on GitHub or check the troubleshooting section above.
