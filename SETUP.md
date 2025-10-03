# Quick Setup Guide

This guide will get you up and running in 5-10 minutes!

## Prerequisites

- LilyGo T-Display S3 device
- USB-C cable
- Computer with WiFi access
- OpenWeatherMap API key (free)

## Step 1: Install PlatformIO

### macOS/Linux
```bash
pip install platformio
```

### Windows
```bash
python -m pip install platformio
```

Or use the [PlatformIO IDE](https://platformio.org/platformio-ide) (VSCode extension)

## Step 2: Get Your API Key

1. Go to https://openweathermap.org/api
2. Click "Sign Up" (top right)
3. Verify your email
4. Go to https://home.openweathermap.org/api_keys
5. Copy your API key (or generate one)

**Note**: New API keys can take 10-15 minutes to activate.

## Step 3: Configure the Project

### 3a. Set WiFi Credentials

Edit `src/pin_config.h` (lines 3-4):
```cpp
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"
```

**Important**: Use your 2.4GHz network (ESP32 doesn't support 5GHz)

### 3b. Set API Key and Location

Edit `src/main.cpp` (lines 14-17):
```cpp
#define OPENWEATHER_API_KEY "YOUR_API_KEY_HERE"
#define LATITUDE "YOUR_LATITUDE"
#define LONGITUDE "YOUR_LONGITUDE"
```

**How to find your coordinates:**
1. Go to [Google Maps](https://maps.google.com)
2. Right-click on your location
3. Click on the coordinates to copy them
4. Or use [LatLong.net](https://www.latlong.net/)

**Examples**:
- Seattle, WA: `"47.6062", "-122.3321"`
- London, UK: `"51.5074", "-0.1278"`
- Tokyo, Japan: `"35.6762", "139.6503"`

**Why coordinates?**
- More accurate than city names, especially for small towns
- No ambiguity (many cities share the same name)
- Better data quality from OpenWeatherMap

## Step 4: Connect and Upload

1. **Connect** your T-Display S3 to your computer via USB-C
2. **Navigate** to the project folder:
   ```bash
   cd lilygo-weather-station
   ```
3. **Upload** the program:
   ```bash
   pio run --target upload
   ```

### If Upload Fails

Put the device in **programming mode**:
1. Hold down the **BOOT** button
2. Press and release the **RESET** button
3. Release the **BOOT** button
4. Try the upload command again

## Step 5: Monitor (Optional)

Watch what's happening:
```bash
pio device monitor --baud 115200
```

Press `Ctrl+C` to exit the monitor.

## Expected Behavior

1. **Immediately**: Display backlight turns on
2. **2-5 seconds**: "Connecting to WiFi..." message
3. **3-10 seconds**: "Connected! Getting weather..."
4. **5-15 seconds**: Weather display appears

## What You'll See

```
┌─────────────────────────────────────┐
│  62°F      Fri       Sat            │  ← Current temp | Next 2 days
│           10-04     10-05            │
│                                      │
│ 72/58°F  68/52°F   61/48°F          │  ← High/Low temps
│                                      │
│ Clear sky Overcast  Scattered       │  ← Conditions
│          clouds     clouds           │
└─────────────────────────────────────┘
```

### Color Key
- **Cyan**: Current temperature and conditions
- **Yellow**: High/Low temps (data is fresh)
- **Red**: High/Low temps with `*` (data is stale, >2 hours old)
- **White**: Day/Date labels
- **Gray**: Future forecast descriptions

## Troubleshooting

### "Failed to connect to WiFi"
- Check SSID and password are correct
- Ensure you're using 2.4GHz network
- Try moving closer to router

### "API Error" or "Failed to get weather"
- Wait 10-15 minutes if API key is brand new
- Verify API key is copied correctly (no extra spaces)
- Check you haven't exceeded free tier (1,000 calls/day)

### Display is blank/black
- Check USB cable is data-capable (not charging-only)
- Try different USB port
- Press RESET button on device

### Wrong location weather
- Double-check `LATITUDE` and `LONGITUDE` in main.cpp
- Make sure coordinates use decimal format (not degrees/minutes/seconds)
- Use negative values for West longitude and South latitude
- Test your coordinates at https://www.openweathermap.org/weathermap

### Compile errors
- Make sure you changed BOTH files:
  - `src/pin_config.h` (WiFi)
  - `src/main.cpp` (API key and location)
- Check for typos in #define statements

## Customization

### Change Update Frequency

Edit `src/main.cpp` (line 19):
```cpp
#define UPDATE_INTERVAL_MS (30 * 60 * 1000)  // 30 minutes
```

Change `30` to any number of minutes you want.

### Switch to Celsius

Edit `src/main.cpp` (line 18):
```cpp
#define UNITS "metric"  // Changes from imperial (Fahrenheit) to metric (Celsius)
```

## Getting Help

1. Check the serial monitor output (`pio device monitor`)
2. Look at the full [README.md](README.md) for detailed docs
3. Verify all settings are correct
4. Try a different city to test if it's location-specific

## Next Steps

Once it's working:
- Unplug from computer and use any USB power source
- Display auto-updates every 30 minutes
- No interaction needed - it just works!

---

**Enjoy your weather station!** 🌤️


