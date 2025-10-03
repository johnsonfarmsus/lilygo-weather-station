#!/bin/bash

echo "🌤️  Lilygo T-Display S3 Weather Station Setup"
echo "=============================================="
echo ""

# Check if config.h exists
if [ ! -f "config.h" ]; then
    echo "❌ config.h not found!"
    exit 1
fi

echo "📝 Current configuration:"
echo "   WiFi SSID: JFTM"
echo "   WiFi Password: howdyhowdyhowdy"
echo "   City: Harrington,WA,US"
echo ""

# Check if API key is set
if grep -q "YOUR_API_KEY_HERE" config.h; then
    echo "🔑 OpenWeatherMap API Key Setup Required"
    echo "========================================"
    echo ""
    echo "1. Go to: https://home.openweathermap.org/users/sign_up"
    echo "2. Create a free account"
    echo "3. Go to API keys section"
    echo "4. Copy your API key"
    echo ""
    read -p "Enter your OpenWeatherMap API key: " api_key
    
    if [ ! -z "$api_key" ]; then
        # Replace the API key in config.h
        sed -i.bak "s/YOUR_API_KEY_HERE/$api_key/g" config.h
        echo "✅ API key updated in config.h"
    else
        echo "❌ No API key provided. Please run setup again."
        exit 1
    fi
else
    echo "✅ API key already configured"
fi

echo ""
echo "🔧 Compiling and uploading to T-Display S3..."
echo ""

# Compile the project
if pio run; then
    echo "✅ Compilation successful"
    
    # Check if device is connected
    if pio device list | grep -q "usbmodem"; then
        echo "📱 T-Display S3 detected, uploading..."
        if pio run --target upload; then
            echo "✅ Upload successful!"
            echo ""
            echo "🎉 Setup complete! Your T-Display S3 should now show:"
            echo "   - Real weather data for Harrington, WA"
            echo "   - 3-day forecast"
            echo "   - Updates every 30 minutes"
            echo ""
            echo "📊 To monitor the device, run: pio device monitor"
        else
            echo "❌ Upload failed. Check USB connection."
        fi
    else
        echo "❌ T-Display S3 not detected. Please:"
        echo "   1. Connect the device via USB-C"
        echo "   2. Run: pio run --target upload"
    fi
else
    echo "❌ Compilation failed. Check the code."
fi
