# LilyGo T-Display S3 Weather Station

A beautiful, real-time weather display showing current conditions and 3-day forecast for any location using the LilyGo T-Display S3 and OpenWeatherMap API.

![T-Display S3 Weather Station](https://img.shields.io/badge/Platform-ESP32--S3-blue) ![License](https://img.shields.io/badge/License-MIT-green)

## Features

- 🌡️ **Current Temperature** - Large, easy-to-read display in cyan
- 📊 **3-Day Forecast** - Today's high/low plus next 2 days
- 🌤️ **Weather Conditions** - Real-time weather descriptions
- 🔄 **Auto-Update** - Refreshes every 30 minutes
- ⚠️ **Stale Data Warning** - Red asterisk if data is >2 hours old
- 🎨 **Clean UI** - Horizontal tile layout with optimized fonts
- 📱 **WiFi Connected** - Automatic connection on startup

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

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/lilygo-weather-station.git
cd lilygo-weather-station
```

### 2. Get Your OpenWeatherMap API Key

1. Sign up at [OpenWeatherMap](https://home.openweathermap.org/users/sign_up)
2. Navigate to **API Keys** in your account
3. Copy your API key (or generate a new one)
4. Note: Free tier allows 1,000 calls/day (plenty for 30-min updates)

### 3. Configure Your Settings

Edit `src/pin_config.h` and update these lines:

```cpp
// WiFi Settings
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"
```

Edit `src/main.cpp` and update the API key (line 11):

```cpp
#define OPENWEATHER_API_KEY "YOUR_API_KEY_HERE"
#define CITY_NAME "YourCity"
#define STATE_CODE "YourState"
#define COUNTRY_CODE "US"
```

### 4. Upload to Device

**Using PlatformIO (Recommended):**

```bash
# Install PlatformIO if you haven't already
pip install platformio

# Connect your T-Display S3 via USB-C
# Compile and upload
pio run --target upload

# Optional: Monitor serial output
pio device monitor --baud 115200
```

**Using Arduino IDE:**

1. Install ESP32 board support
2. Select board: **ESP32S3 Dev Module** or **LilyGo T-Display-S3**
3. Copy all files to Arduino sketch folder
4. Install required libraries (LVGL 8.3+, ArduinoJson 7.0+)
5. Upload to board

## Configuration Options

### Update Frequency

Change how often weather data refreshes (line 17 in `main.cpp`):

```cpp
#define UPDATE_INTERVAL_MS (30 * 60 * 1000)  // 30 minutes (default)
```

### Temperature Units

Switch between Fahrenheit and Celsius (line 15 in `main.cpp`):

```cpp
#define UNITS "imperial"  // Fahrenheit
// or
#define UNITS "metric"    // Celsius
```

### Location

Update your city (lines 12-14 in `main.cpp`):

```cpp
#define CITY_NAME "YourCity"
#define STATE_CODE "YourState"  // US only
#define COUNTRY_CODE "US"
```

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

### Display Not Working
- Ensure GPIO15 is powered (handled automatically in code)
- The T-Display S3 uses **parallel I80 interface**, not SPI
- Check that LVGL is properly configured

### WiFi Connection Issues
- Verify SSID and password in `pin_config.h`
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Serial monitor will show connection status

### API Errors
- Verify API key is correct and activated (can take a few minutes after creation)
- Check you haven't exceeded free tier limits (1,000 calls/day)
- Ensure city name format is correct: `"City,StateCode,CountryCode"`

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

## Technical Details

### Libraries Used
- **LVGL 8.3+** - Graphics library for display
- **ArduinoJson 7.0+** - JSON parsing for API responses
- **WiFi** - ESP32 WiFi connection
- **HTTPClient** - API requests

### Display Specs
- **Controller**: ST7789V
- **Resolution**: 320x170 pixels
- **Interface**: 8-bit parallel I80
- **Backlight**: GPIO38
- **Power**: GPIO15

### API Endpoint
Uses OpenWeatherMap's 5-day/3-hour forecast API:
```
http://api.openweathermap.org/data/2.5/forecast
```

### Memory Usage
- **RAM**: ~19KB (6% of 327KB)
- **Flash**: ~1.1MB (17% of 6.5MB)

## Project Structure

```
lilygo-weather-station/
├── src/
│   ├── main.cpp              # Main application code
│   ├── pin_config.h          # Pin definitions and WiFi config
│   └── zones.h               # Timezone data
├── lib/
│   └── lv_conf.h             # LVGL configuration
├── platformio.ini            # PlatformIO configuration
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

### v1.0.0 (Current)
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
- Multiple location support
- Touch screen controls
- Battery level indicator
- Dark/light mode toggle

---

**Need help?** Open an issue on GitHub or check the troubleshooting section above.
