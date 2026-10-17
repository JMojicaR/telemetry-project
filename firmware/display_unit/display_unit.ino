// ============================================================================
// Bicycle display unit — ESP32-S3 SuperMini + 2.4" ILI9341 SPI TFT
// ----------------------------------------------------------------------------
// Receives distance estimates from 2 ESP32-CAM nodes over ESP-NOW and shows a
// color-coded rear-view dashboard:
//   - "VISTA AMPLIA"  (wide-angle rear, approaching cars)
//   - "RANGO CORTO"   (close-range rear, blind spot)
// Color: GREEN > 5 m · YELLOW 2–5 m · RED < 2 m · GRAY = no signal.
// (On-screen warning only — per design, no buzzer.)
// ============================================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ---- ILI9341 SPI pins (ESP32-S3 SuperMini — VERIFY against your board) ----
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_BL     7   // backlight (optional — tie to 3.3 V if unused)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ---- ESP-NOW message (MUST match cam_node.ino) ----
typedef struct {
    uint8_t cam_id;
    float   distance_m;
    int16_t obj_w;
    int16_t obj_cx;
    int16_t obj_cy;
} cam_msg_t;

#define FAR_M   5.0f
#define NEAR_M  2.0f
#define STALE_MS 3000UL

static float         wideDist  = -1.0f;  // -1 = no data yet
static float         closeDist = -1.0f;
static unsigned long lastWide  = 0;
static unsigned long lastClose = 0;

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    (void)info; // sender MAC available as info->src_addr if needed
    if (len != sizeof(cam_msg_t)) return;
    cam_msg_t m;
    memcpy(&m, data, len);
    if (m.cam_id == 0) { wideDist  = m.distance_m; lastWide  = millis(); }
    else               { closeDist = m.distance_m; lastClose = millis(); }
}

uint16_t statusColor(float d, unsigned long last) {
    if (millis() - last > STALE_MS || d < 0.0f) return 0x7BEF; // gray
    if (d < NEAR_M) return 0xF800;   // red
    if (d < FAR_M)  return 0xFFE0;   // yellow
    return 0x07E0;                   // green
}

void drawPanel(int y, int h, const char *title, float dist, unsigned long last) {
    uint16_t c = statusColor(dist, last);
    tft.fillRect(0, y, 240, h, c);

    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, y + 10);
    tft.print(title);

    tft.setTextSize(3);
    tft.setCursor(10, y + h - 60);
    if (dist < 0.0f) {
        tft.print("--  m");
    } else {
        tft.print(dist, 1);
        tft.print(" m");
    }

    tft.setTextSize(1);
    tft.setCursor(10, y + h - 16);
    if (millis() - last > STALE_MS)      tft.print("SIN SENAL");
    else if (dist >= FAR_M)              tft.print("SEGURO");
    else if (dist >= NEAR_M)             tft.print("ATENCION");
    else if (dist >= 0.0f)               tft.print("PELIGRO");
}

void setup() {
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(0);            // portrait 240x320
    tft.fillScreen(ILI9341_BLACK);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // match the CAM nodes
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("Display unit ready (ESP-NOW, ch 1)");

    // Title bar
    tft.fillRect(0, 0, 240, 20, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(8, 6);
    tft.print("BICI RETROVISOR");
}

void loop() {
    drawPanel(20, 150, "VISTA AMPLIA", wideDist, lastWide);
    drawPanel(170, 150, "RANGO CORTO", closeDist, lastClose);
    delay(150);   // ~6–7 fps redraw
}
