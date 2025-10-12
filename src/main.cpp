#include "Arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "pin_config.h"
#include <time.h>
#include <WifiLocation.h>

// Import API keys and credentials from secrets.h
// Copy secrets.h.template to secrets.h and fill in your values
#include "../secrets.h"

// Weather Configuration
#define UNITS "imperial"
#define UPDATE_INTERVAL_MS (30 * 60 * 1000)  // 30 minutes

// Display handles
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t *lv_disp_buf;
static bool is_initialized_lvgl = false;

// Weather data
struct WeatherDay {
    String date;
    String day_name;
    String description;
    int temp_high;
    int temp_low;
    int humidity;
};

WeatherDay forecast[3];
int currentTemp = 0;  // Current temperature for Day 0
String currentCondition = "";  // Current weather condition for Day 0
unsigned long lastUpdate = 0;
bool weatherDataValid = false;
long timezoneOffset = 0;  // Timezone offset in seconds from UTC (from API)

// Location tracking for WiFi triangulation
struct Location {
    float latitude;
    float longitude;
    String city;
    unsigned long lastLocationCheck;
    bool isValid;
};

Location currentLocation = {0.0, 0.0, "", 0, false};
const float LOCATION_CHANGE_THRESHOLD_KM = 5.0; // Only update weather if moved >5km

// LVGL UI objects
lv_obj_t *screen;
lv_obj_t *title_label;
lv_obj_t *day_labels[3];
lv_obj_t *temp_labels[3];
lv_obj_t *desc_labels[3];
lv_obj_t *update_label;

// LCD commands
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};

// Function declarations
void initDisplay();
bool connectToWiFi();
bool fetchWeatherData();
void createUI();
void updateWeatherDisplay();
String getDayName(const String& dateStr);
static bool lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// Location and security functions
void validateAPIKeys();
bool getLocationFromWiFi();
float calculateDistance(float lat1, float lon1, float lat2, float lon2);
bool locationChanged(float newLat, float newLon);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("LilyGo Weather Station v1.3.0");
    Serial.println("With WiFi Triangulation");
    Serial.println("========================================\n");

    // Validate API keys first (will halt if not configured)
    validateAPIKeys();

    // Power on display
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    delay(100);

    // Initialize display and LVGL
    initDisplay();

    // Create UI
    createUI();

    lv_label_set_text(title_label, "Connecting WiFi...");
    lv_timer_handler();
    
    // Connect to WiFi
    if (connectToWiFi()) {
        Serial.println("WiFi connected!");

        // Get location via WiFi triangulation
        lv_label_set_text(title_label, "Finding Location...");
        lv_timer_handler();
        Serial.println("\n--- Location Detection ---");

        if (getLocationFromWiFi()) {
            Serial.println("✓ Location acquired successfully");
            Serial.printf("Coordinates: %.6f, %.6f\n",
                         currentLocation.latitude, currentLocation.longitude);
        } else {
            Serial.println("✗ Location detection failed!");
            lv_label_set_text(title_label, "Location Failed!");
            // Don't proceed without location
            while(1) { delay(1000); }
        }

        // Configure NTP for UTC time (we'll apply timezone offset from API)
        lv_label_set_text(title_label, "Syncing Time...");
        lv_timer_handler();
        configTime(0, 0, NTP_SERVER1, NTP_SERVER2);  // UTC timezone

        // Wait for time sync
        Serial.println("\n--- Time Synchronization ---");
        Serial.println("Waiting for NTP time sync...");
        int timeWait = 0;
        while (time(nullptr) < 100000 && timeWait < 20) {
            delay(500);
            Serial.print(".");
            timeWait++;
        }
        Serial.println("\n✓ Time synced!");

        lv_label_set_text(title_label, "Fetching Weather...");
        lv_timer_handler();
        delay(1000);

        // Fetch weather (this will also get timezone offset from API)
        Serial.println("\n--- Weather Data ---");
        if (fetchWeatherData()) {
            weatherDataValid = true;
            updateWeatherDisplay();
            Serial.println("✓ Weather data loaded successfully\n");
        } else {
            lv_label_set_text(title_label, "Weather Fetch Failed");
            Serial.println("✗ Weather fetch failed\n");
        }
    } else {
        lv_label_set_text(title_label, "WiFi Failed!");
        Serial.println("✗ WiFi connection failed\n");
    }
    
    // Turn on backlight
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);
}

void loop() {
    lv_timer_handler();
    delay(5);

    // Update weather every 30 minutes
    // Check location BEFORE each weather update
    if (weatherDataValid && millis() - lastUpdate > UPDATE_INTERVAL_MS) {
        Serial.println("\n--- Periodic Weather Update ---");
        Serial.println("Checking location before weather update...");

        // Triangulate to see if location changed
        if (getLocationFromWiFi()) {
            Serial.println("Location changed significantly! Updating weather for new location...");
            // Location changed >5km, fetch weather for new location
            if (fetchWeatherData()) {
                updateWeatherDisplay();
                Serial.println("✓ Weather updated for new location\n");
            } else {
                Serial.println("✗ Weather update failed\n");
            }
        } else {
            // Location unchanged (<5km difference) or triangulation returned false
            // Still update weather for current location
            Serial.println("Location unchanged. Refreshing weather for current location...");
            if (fetchWeatherData()) {
                updateWeatherDisplay();
                Serial.println("✓ Weather data refreshed\n");
            } else {
                Serial.println("✗ Weather update failed\n");
            }
        }
    }
}

static bool lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    if (is_initialized_lvgl) {
        lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
        lv_disp_flush_ready(disp_driver);
    }
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}

void initDisplay() {
    pinMode(PIN_LCD_RD, OUTPUT);
    digitalWrite(PIN_LCD_RD, HIGH);
    
    // Initialize I80 bus
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .wr_gpio_num = PIN_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums = {
            PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3,
            PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LVGL_LCD_BUF_SIZE * sizeof(uint16_t),
        .psram_trans_align = 0,
        .sram_trans_align = 0
    };
    esp_lcd_new_i80_bus(&bus_config, &i80_bus);
    
    // Initialize panel IO
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 20,
        .on_color_trans_done = lvgl_flush_ready,
        .user_ctx = &disp_drv,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
    };
    esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);
    
    // Initialize panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RES,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 35);
    
    // Send ST7789 specific commands
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        esp_lcd_panel_io_tx_param(io_handle, lcd_st7789v[i].cmd, lcd_st7789v[i].data, lcd_st7789v[i].len & 0x7f);
        if (lcd_st7789v[i].len & 0x80) delay(120);
    }
    
    // Initialize LVGL
    lv_init();
    lv_disp_buf = (lv_color_t *)heap_caps_malloc(LVGL_LCD_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    lv_disp_draw_buf_init(&disp_buf, lv_disp_buf, NULL, LVGL_LCD_BUF_SIZE);
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);
    
    is_initialized_lvgl = true;
}

void createUI() {
    screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    
    // Title (used for status messages during startup)
    title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "Weather Station");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    
    // Create 3 horizontal weather tiles
    int tile_width = 100;
    int tile_spacing = 6;
    int start_x = (320 - (tile_width * 3 + tile_spacing * 2)) / 2;
    
    for (int i = 0; i < 3; i++) {
        int x_pos = start_x + (i * (tile_width + tile_spacing));
        
        // Day name label (at top of tile)
        day_labels[i] = lv_label_create(screen);
        lv_obj_set_style_text_color(day_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(day_labels[i], &lv_font_montserrat_12, 0);
        lv_obj_set_pos(day_labels[i], x_pos, 10);
        lv_obj_set_width(day_labels[i], tile_width);
        lv_obj_set_style_text_align(day_labels[i], LV_TEXT_ALIGN_CENTER, 0);
        
        // Temperature label (LARGE, moved down slightly)
        temp_labels[i] = lv_label_create(screen);
        lv_obj_set_style_text_color(temp_labels[i], lv_color_hex(0xFFFF00), 0);
        lv_obj_set_style_text_font(temp_labels[i], &lv_font_montserrat_24, 0);
        lv_obj_set_pos(temp_labels[i], x_pos, 65);
        lv_obj_set_width(temp_labels[i], tile_width);
        lv_obj_set_style_text_align(temp_labels[i], LV_TEXT_ALIGN_CENTER, 0);
        
        // Description label (near bottom, multi-line, size 18)
        desc_labels[i] = lv_label_create(screen);
        lv_obj_set_style_text_color(desc_labels[i], lv_color_hex(0xAAAAAA), 0);
        lv_obj_set_style_text_font(desc_labels[i], &lv_font_montserrat_18, 0);
        lv_obj_set_pos(desc_labels[i], x_pos, 120);
        lv_obj_set_width(desc_labels[i], tile_width);
        lv_obj_set_style_text_align(desc_labels[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(desc_labels[i], LV_LABEL_LONG_WRAP);
    }
    
    // Update label - initially hidden, will be used for status messages
    update_label = lv_label_create(screen);
    lv_obj_set_style_text_color(update_label, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(update_label, &lv_font_montserrat_14, 0);
    lv_obj_align(update_label, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_add_flag(update_label, LV_OBJ_FLAG_HIDDEN);
}

void updateWeatherDisplay() {
    // Hide startup message
    lv_obj_add_flag(title_label, LV_OBJ_FLAG_HIDDEN);
    
    // Check if data is stale (older than 2 hours)
    bool isStale = (millis() - lastUpdate) > (2 * 60 * 60 * 1000);
    
    for (int i = 0; i < 3; i++) {
        // Day 0: Show current temp instead of day/date
        // Days 1-2: Show day name and date
        if (i == 0) {
            // Current temperature displayed where day/date usually goes
            String current_text = String(currentTemp) + "F";
            lv_label_set_text(day_labels[i], current_text.c_str());
            lv_obj_set_style_text_color(day_labels[i], lv_color_hex(0x00FFFF), 0); // Cyan for current
            lv_obj_set_style_text_font(day_labels[i], &lv_font_montserrat_26, 0); // Larger than temps
        } else {
            // Normal day/date display
            String day_text = forecast[i].day_name + "\n" + forecast[i].date;
            lv_label_set_text(day_labels[i], day_text.c_str());
            lv_obj_set_style_text_color(day_labels[i], lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(day_labels[i], &lv_font_montserrat_12, 0);
        }
        
        // Temperature with red asterisk if stale
        String temp_text;
        if (isStale) {
            temp_text = "*" + String(forecast[i].temp_high) + "/" + String(forecast[i].temp_low) + "F";
            lv_obj_set_style_text_color(temp_labels[i], lv_color_hex(0xFF0000), 0); // Red when stale
        } else {
            temp_text = String(forecast[i].temp_high) + "/" + String(forecast[i].temp_low) + "F";
            lv_obj_set_style_text_color(temp_labels[i], lv_color_hex(0xFFFF00), 0); // Yellow when fresh
        }
        lv_label_set_text(temp_labels[i], temp_text.c_str());
        
        // Description
        String desc;
        if (i == 0) {
            // Day 0: Show current conditions in cyan
            desc = currentCondition;
            if (desc.length() > 0) {
                desc[0] = toupper(desc[0]);
            }
            lv_label_set_text(desc_labels[i], desc.c_str());
            lv_obj_set_style_text_color(desc_labels[i], lv_color_hex(0x00FFFF), 0); // Cyan to match current temp
        } else {
            // Days 1-2: Show forecast in gray
            desc = forecast[i].description;
            if (desc.length() > 0) {
                desc[0] = toupper(desc[0]);
            }
            lv_label_set_text(desc_labels[i], desc.c_str());
            lv_obj_set_style_text_color(desc_labels[i], lv_color_hex(0xAAAAAA), 0); // Gray
        }
    }
}

bool connectToWiFi() {
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    return (WiFi.status() == WL_CONNECTED);
}

bool fetchWeatherData() {
    if (WiFi.status() != WL_CONNECTED) return false;

    // Check if we have a valid location
    if (!currentLocation.isValid) {
        Serial.println("ERROR: No valid location available for weather fetch");
        return false;
    }

    HTTPClient http;

    // STEP 1: Get ACTUAL current weather using dynamic coordinates
    String currentUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" +
                        String(currentLocation.latitude, 6) + "&lon=" +
                        String(currentLocation.longitude, 6) +
                        "&appid=" + String(OPENWEATHER_API_KEY) +
                        "&units=" + String(UNITS);
    
    Serial.println("Fetching current weather...");
    http.begin(currentUrl);
    int httpCode = http.GET();
    
    if (httpCode != 200) {
        Serial.printf("Current weather HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }
    
    String currentPayload = http.getString();
    http.end();
    
    JsonDocument currentDoc;
    DeserializationError error = deserializeJson(currentDoc, currentPayload);
    
    if (error) {
        Serial.print("Current weather JSON error: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Get location name returned by API to verify we have the right place
    String locationName = currentDoc["name"].as<String>();
    currentLocation.city = locationName;  // Store city name
    Serial.printf("Location: %s (%.6f, %.6f)\n", locationName.c_str(),
                 currentLocation.latitude, currentLocation.longitude);

    // Get current conditions (but we'll use forecast temp instead)
    currentCondition = currentDoc["weather"][0]["description"].as<String>();

    Serial.printf("Current Weather API conditions: %s\n", currentCondition.c_str());

    // STEP 2: Get forecast data using dynamic coordinates
    String forecastUrl = "http://api.openweathermap.org/data/2.5/forecast?lat=" +
                         String(currentLocation.latitude, 6) + "&lon=" +
                         String(currentLocation.longitude, 6) +
                         "&appid=" + String(OPENWEATHER_API_KEY) +
                         "&units=" + String(UNITS) + "&cnt=40";  // Get 5 days worth
    
    Serial.println("Fetching forecast...");
    http.begin(forecastUrl);
    httpCode = http.GET();
    
    if (httpCode != 200) {
        Serial.printf("Forecast HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }
    
    String payload = http.getString();
    http.end();
    
    JsonDocument doc;
    error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("Forecast JSON error: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Get timezone offset from API (in seconds from UTC)
    timezoneOffset = doc["city"]["timezone"].as<long>();
    Serial.printf("Timezone offset from API: %ld seconds (%d hours)\n", timezoneOffset, (int)(timezoneOffset / 3600));
    
    // Get the first forecast entry temperature (most recent/next 3-hour block)
    int firstForecastTemp = round(doc["list"][0]["main"]["temp"].as<float>());
    Serial.printf("First Forecast Entry: %dF\n", firstForecastTemp);
    
    // Use the forecast temperature if it's more recent (for small towns, forecast is more reliable)
    currentTemp = firstForecastTemp;
    
    // Process forecast data - group by actual calendar date
    Serial.println("Processing forecast by calendar date...");
    
    // Get current local date using timezone offset from API
    time_t now = time(nullptr);
    time_t localTime = now + timezoneOffset;
    struct tm* timeinfo = gmtime(&localTime);
    char currentDate[11];
    strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", timeinfo);
    Serial.printf("Local date for grouping: %s (UTC+%d)\n", currentDate, (int)(timezoneOffset / 3600));
    
    // Arrays to hold min/max for each of 3 days
    float dayHighs[3] = {-999, -999, -999};
    float dayLows[3] = {999, 999, 999};
    String dayDescriptions[3] = {"", "", ""};
    String dayDates[3] = {"", "", ""};
    int dayHumiditySums[3] = {0, 0, 0};
    int dayHumidityCounts[3] = {0, 0, 0};
    
    // Process all forecast entries and group by date
    int listSize = doc["list"].size();
    for (int i = 0; i < listSize; i++) {
        JsonObject item = doc["list"][i];
        
        // Get Unix timestamp (in UTC) and convert to local time using API timezone offset
        long dt = item["dt"].as<long>();
        time_t forecast_time = (time_t)(dt + timezoneOffset);
        
        // Convert to local date string
        struct tm* forecast_tm = gmtime(&forecast_time);
        char forecastDate[11];
        strftime(forecastDate, sizeof(forecastDate), "%Y-%m-%d", forecast_tm);
        char forecastTime[6];
        strftime(forecastTime, sizeof(forecastTime), "%H:%M", forecast_tm);
        
        // Determine which day this forecast belongs to (0=today, 1=tomorrow, 2=day after)
        int dayIndex = -1;
        
        // Compare dates to determine day index
        if (strcmp(forecastDate, currentDate) == 0) {
            dayIndex = 0;  // Today
        } else if (dayDates[1] == "" || dayDates[1] == forecastDate) {
            dayIndex = 1;  // Tomorrow
            if (dayDates[1] == "") dayDates[1] = forecastDate;
        } else if (dayDates[2] == "" || dayDates[2] == forecastDate) {
            dayIndex = 2;  // Day after tomorrow
            if (dayDates[2] == "") dayDates[2] = forecastDate;
        } else {
            continue;  // Skip if beyond 3 days
        }
        
        // Store date for this day
        if (dayDates[dayIndex] == "") {
            dayDates[dayIndex] = forecastDate;
        }
        
        // Get temperature and update min/max
        float temp = item["main"]["temp"].as<float>();
        if (temp > dayHighs[dayIndex]) {
            dayHighs[dayIndex] = temp;
        }
        if (temp < dayLows[dayIndex]) {
            dayLows[dayIndex] = temp;
        }
        
        // Accumulate humidity
        dayHumiditySums[dayIndex] += item["main"]["humidity"].as<int>();
        dayHumidityCounts[dayIndex]++;
        
        // Use midday description (12:00-15:00 preferred)
        if (dayDescriptions[dayIndex] == "" || 
            (strcmp(forecastTime, "12:00") >= 0 && strcmp(forecastTime, "15:00") <= 0)) {
            dayDescriptions[dayIndex] = item["weather"][0]["description"].as<String>();
        }
    }
    
    // Populate forecast array with processed data
    dayDates[0] = currentDate;  // Ensure today is set
    for (int day = 0; day < 3; day++) {
        forecast[day].day_name = getDayName(dayDates[day]);
        forecast[day].date = dayDates[day].substring(5, 10);  // "MM-DD"
        forecast[day].description = dayDescriptions[day];
        
        // Handle missing or partial forecast data
        if (dayHighs[day] <= -999 || dayLows[day] >= 999) {
            Serial.printf("No forecast data for day %d\n", day);
            // For today (day 0), if we have no forecast entries remaining,
            // treat current temp as the high since it's likely late in the day
            if (day == 0) {
                forecast[day].temp_high = currentTemp;
                forecast[day].temp_low = currentTemp;
                Serial.printf("Late in day - using current temp for day 0: %d/%dF\n", currentTemp, currentTemp);
            } else {
                // For future days, show 0/0 to indicate no data
                forecast[day].temp_high = 0;
                forecast[day].temp_low = 0;
            }
        } else {
            // We have forecast data - use it
            forecast[day].temp_high = round(dayHighs[day]);
            forecast[day].temp_low = round(dayLows[day]);
            
            // Special handling for Day 0: if current temp is higher than forecast high,
            // use current temp as the high (in case we're past the forecasted peak)
            if (day == 0 && currentTemp > forecast[day].temp_high) {
                Serial.printf("Current temp (%dF) exceeds forecast high (%dF), using current as high\n", 
                             currentTemp, forecast[day].temp_high);
                forecast[day].temp_high = currentTemp;
            }
            // Similarly, if current temp is lower than forecast low, use current as low
            if (day == 0 && currentTemp < forecast[day].temp_low) {
                Serial.printf("Current temp (%dF) below forecast low (%dF), using current as low\n", 
                             currentTemp, forecast[day].temp_low);
                forecast[day].temp_low = currentTemp;
            }
        }
        
        forecast[day].humidity = dayHumidityCounts[day] > 0 ? 
                                 dayHumiditySums[day] / dayHumidityCounts[day] : 0;
        
        Serial.printf("Day %d: %s %s, %d/%dF, %s\n", 
                     day, forecast[day].day_name.c_str(), forecast[day].date.c_str(),
                     forecast[day].temp_low, forecast[day].temp_high,
                     forecast[day].description.c_str());
    }
    
    lastUpdate = millis();
    return true;
}

String getDayName(const String& dateStr) {
    // dateStr format: "YYYY-MM-DD"
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    // Parse the date string
    int year = dateStr.substring(0, 4).toInt();
    int month = dateStr.substring(5, 7).toInt();
    int day = dateStr.substring(8, 10).toInt();

    // Create tm struct
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;

    // Convert to time_t and back to get day of week
    time_t timestamp = mktime(&timeinfo);
    struct tm* result = localtime(&timestamp);

    return String(days[result->tm_wday]);
}

// ========================================
// API Key Validation
// ========================================
void validateAPIKeys() {
    Serial.println("Validating API keys...");

    // Check if secrets.h exists and keys are set
    #ifndef OPENWEATHER_API_KEY
        #error "secrets.h file missing - copy from secrets.h.template and configure your API keys"
    #endif

    #ifndef GOOGLE_GEOLOCATION_API_KEY
        #error "secrets.h file missing - copy from secrets.h.template and configure your API keys"
    #endif

    #ifndef WIFI_SSID
        #error "secrets.h file missing - WiFi credentials not defined"
    #endif

    // Check if default placeholder values are still in use
    if (strcmp(OPENWEATHER_API_KEY, "your_openweather_api_key_here") == 0) {
        Serial.println("\n========================================");
        Serial.println("ERROR: OpenWeatherMap API key not configured!");
        Serial.println("========================================");
        Serial.println("Please follow these steps:");
        Serial.println("1. Copy secrets.h.template to secrets.h");
        Serial.println("2. Edit secrets.h and add your actual API keys");
        Serial.println("3. Get a free API key at: https://openweathermap.org/api");
        Serial.println("========================================\n");
        while(1) { delay(1000); } // Halt execution
    }

    if (strcmp(GOOGLE_GEOLOCATION_API_KEY, "your_google_geolocation_api_key_here") == 0) {
        Serial.println("\n========================================");
        Serial.println("ERROR: Google Geolocation API key not configured!");
        Serial.println("========================================");
        Serial.println("Please follow these steps:");
        Serial.println("1. Copy secrets.h.template to secrets.h");
        Serial.println("2. Edit secrets.h and add your actual API keys");
        Serial.println("3. Get API key at: https://console.cloud.google.com/");
        Serial.println("========================================\n");
        while(1) { delay(1000); } // Halt execution
    }

    if (strcmp(WIFI_SSID, "your_wifi_ssid_here") == 0) {
        Serial.println("\n========================================");
        Serial.println("ERROR: WiFi credentials not configured!");
        Serial.println("========================================");
        Serial.println("Please edit secrets.h and add your WiFi SSID and password");
        Serial.println("========================================\n");
        while(1) { delay(1000); } // Halt execution
    }

    Serial.println("✓ API keys validated successfully");
}

// ========================================
// WiFi Triangulation & Location Functions
// ========================================
bool getLocationFromWiFi() {
    Serial.println("Getting location via WiFi triangulation...");

    // Check if user wants to always use fallback location
    #ifdef USE_FALLBACK_LOCATION
    if (USE_FALLBACK_LOCATION) {
        Serial.println("Using fallback location (USE_FALLBACK_LOCATION = true)");

        #ifdef FALLBACK_LATITUDE
        #ifdef FALLBACK_LONGITUDE
        if (strcmp(FALLBACK_LATITUDE, "your_latitude_here") != 0 &&
            strcmp(FALLBACK_LONGITUDE, "your_longitude_here") != 0) {
            currentLocation.latitude = String(FALLBACK_LATITUDE).toFloat();
            currentLocation.longitude = String(FALLBACK_LONGITUDE).toFloat();
            currentLocation.lastLocationCheck = millis();
            currentLocation.isValid = true;
            Serial.printf("Fallback location set: %f, %f\n",
                         currentLocation.latitude, currentLocation.longitude);
            return true;
        }
        #endif
        #endif

        Serial.println("ERROR: Fallback location not configured in secrets.h!");
        return false;
    }
    #endif

    // Try WiFi triangulation
    WifiLocation location(GOOGLE_GEOLOCATION_API_KEY);

    location_t loc = location.getGeoFromWiFi();

    if (loc.accuracy > 0) {
        float newLat = loc.lat;
        float newLon = loc.lon;

        Serial.println("✓ WiFi TRIANGULATION SUCCESSFUL!");
        Serial.printf("   Location: %f, %f (accuracy: %.0fm)\n",
                     newLat, newLon, loc.accuracy);

        // Check if location changed significantly
        if (currentLocation.isValid && !locationChanged(newLat, newLon)) {
            Serial.println("Location hasn't changed significantly, skipping weather update");
            currentLocation.lastLocationCheck = millis();
            return false; // Location didn't change enough to warrant update
        }

        // Update location
        currentLocation.latitude = newLat;
        currentLocation.longitude = newLon;
        currentLocation.lastLocationCheck = millis();
        currentLocation.isValid = true;

        return true; // Location updated successfully
    } else {
        Serial.println("✗ WiFi TRIANGULATION FAILED!");
        Serial.printf("   API returned accuracy: %.0fm (need >0 for success)\n", loc.accuracy);

        // Try fallback location
        #ifdef FALLBACK_LATITUDE
        #ifdef FALLBACK_LONGITUDE
        if (!currentLocation.isValid &&
            strcmp(FALLBACK_LATITUDE, "your_latitude_here") != 0 &&
            strcmp(FALLBACK_LONGITUDE, "your_longitude_here") != 0) {
            Serial.println("→ USING FALLBACK LOCATION (from secrets.h)");
            currentLocation.latitude = String(FALLBACK_LATITUDE).toFloat();
            currentLocation.longitude = String(FALLBACK_LONGITUDE).toFloat();
            currentLocation.lastLocationCheck = millis();
            currentLocation.isValid = true;
            Serial.printf("   Fallback coordinates: %f, %f\n",
                         currentLocation.latitude, currentLocation.longitude);
            return true;
        }
        #endif
        #endif

        return false;
    }
}

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    // Haversine formula to calculate distance between two coordinates in kilometers
    const float R = 6371.0; // Earth radius in km

    float dLat = (lat2 - lat1) * PI / 180.0;
    float dLon = (lon2 - lon1) * PI / 180.0;

    float a = sin(dLat/2) * sin(dLat/2) +
              cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0) *
              sin(dLon/2) * sin(dLon/2);

    float c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c; // Distance in km
}

bool locationChanged(float newLat, float newLon) {
    float distance = calculateDistance(
        currentLocation.latitude, currentLocation.longitude,
        newLat, newLon
    );

    Serial.printf("Distance from previous location: %.2f km (threshold: %.2f km)\n",
                 distance, LOCATION_CHANGE_THRESHOLD_KM);

    return (distance >= LOCATION_CHANGE_THRESHOLD_KM);
}
