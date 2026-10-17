// ============================================================================
// Bicycle rear-view camera node — ESP32-CAM (AI-Thinker)
// ----------------------------------------------------------------------------
// Captures grayscale frames, detects motion via background subtraction, finds
// the bounding box of the moving object, estimates its DISTANCE with a
// monocular model (known object width), and sends the result to the display
// unit over ESP-NOW.
//
// Two of these run (set CAM_ID: 0 = wide-angle rear, 1 = close-range rear).
//
// DISTANCE MODEL (monocular):
//   distance_m = FOCAL_LENGTH_PX * REAL_WIDTH_M / object_width_px
//   - FOCAL_LENGTH_PX: lens focal length in pixels (calibrate per camera).
//   - REAL_WIDTH_M   : assumed real width of a car (~1.8 m).
//
// NOTE: background subtraction works best when the bike is slow or stopped
// (the whole scene shifts while riding fast, which floods the motion mask).
// For robust moving-vehicle detection see docs/components_bicycle.md (upgrade).
// ============================================================================

#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ---- Camera pins (AI-Thinker ESP32-CAM) ----
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ---- Identity ----
#define CAM_ID            0   // 0 = wide-angle rear, 1 = close-range rear

// ---- CV / distance constants ----
#define FRAME_W           160
#define FRAME_H           120
#define MOTION_THRESHOLD  25     // abs grayscale diff to count a pixel as motion
#define MIN_AREA_PX       150    // min motion area before we report an object
#define FOCAL_LENGTH_PX   125.0f // OV2640 @160px, ~65° HFOV — CALIBRATE
#define REAL_WIDTH_M      1.8f   // assumed width of a car, meters

// ---- ESP-NOW message (MUST match display_unit.ino) ----
typedef struct {
    uint8_t cam_id;      // 0 / 1
    float   distance_m;  // 0 = nothing detected
    int16_t obj_w;       // object width in px
    int16_t obj_cx;      // object center x in px
    int16_t obj_cy;      // object center y in px
} cam_msg_t;

static uint8_t cur_frame[FRAME_W * FRAME_H];   // current grayscale
static uint8_t ref_frame[FRAME_W * FRAME_H];   // background reference
static bool    ref_ready = false;
static uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void initCamera() {
    camera_config_t cfg;
    cfg.ledc_channel = LEDC_CHANNEL_0;
    cfg.ledc_timer   = LEDC_TIMER_0;
    cfg.pin_d0       = Y2_GPIO_NUM;
    cfg.pin_d1       = Y3_GPIO_NUM;
    cfg.pin_d2       = Y4_GPIO_NUM;
    cfg.pin_d3       = Y5_GPIO_NUM;
    cfg.pin_d4       = Y6_GPIO_NUM;
    cfg.pin_d5       = Y7_GPIO_NUM;
    cfg.pin_d6       = Y8_GPIO_NUM;
    cfg.pin_d7       = Y9_GPIO_NUM;
    cfg.pin_xclk     = XCLK_GPIO_NUM;
    cfg.pin_pclk     = PCLK_GPIO_NUM;
    cfg.pin_vsync    = VSYNC_GPIO_NUM;
    cfg.pin_href     = HREF_GPIO_NUM;
    cfg.pin_sscb_sda = SIOD_GPIO_NUM;
    cfg.pin_sscb_scl = SIOC_GPIO_NUM;
    cfg.pin_pwdn     = PWDN_GPIO_NUM;
    cfg.pin_reset    = RESET_GPIO_NUM;
    cfg.xclk_freq_hz = 20000000;
    cfg.pixel_format = PIXFORMAT_GRAYSCALE;
    cfg.frame_size   = FRAMESIZE_QQVGA;   // 160x120
    cfg.fb_count     = 1;
    cfg.grab_mode    = CAMERA_GRAB_LATEST;

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        while (true) delay(1000);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Bicycle cam node (CAM_ID=%d)\n", CAM_ID);

    initCamera();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // match display unit
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcast, 6);
    peer.channel = 1;      // fixed channel — display unit must match
    peer.encrypt = false;
    if (!esp_now_is_peer_exist(broadcast)) {
        esp_now_add_peer(&peer);
    }
    Serial.println("ESP-NOW ready (broadcast, ch 1)");
}

void loop() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { delay(50); return; }
    memcpy(cur_frame, fb->buf, FRAME_W * FRAME_H);
    esp_camera_fb_return(fb);

    if (!ref_ready) {
        memcpy(ref_frame, cur_frame, FRAME_W * FRAME_H);
        ref_ready = true;
        delay(100);
        return;
    }

    // Frame differencing + bounding box of motion.
    int minX = FRAME_W, minY = FRAME_H, maxX = -1, maxY = -1, area = 0;
    for (int y = 0; y < FRAME_H; y++) {
        for (int x = 0; x < FRAME_W; x++) {
            int i = y * FRAME_W + x;
            int d = (int)cur_frame[i] - (int)ref_frame[i];
            if (d < 0) d = -d;
            if (d > MOTION_THRESHOLD) {
                area++;
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }

    // Build + send the message.
    cam_msg_t msg;
    msg.cam_id = CAM_ID;
    if (area > MIN_AREA_PX && maxX > minX) {
        int w = maxX - minX + 1;
        msg.obj_w  = w;
        msg.obj_cx = (minX + maxX) / 2;
        msg.obj_cy = (minY + maxY) / 2;
        msg.distance_m = (FOCAL_LENGTH_PX * REAL_WIDTH_M) / (float)w;
    } else {
        msg.distance_m = 0.0f;
        msg.obj_w  = 0;
        msg.obj_cx = -1;
        msg.obj_cy = -1;
    }

    esp_now_send(broadcast, (uint8_t *)&msg, sizeof(msg));

    // Slowly adapt the background (leaky integrator, alpha ≈ 1/16) so the road
    // doesn't stay "motion" forever, while a fast-approaching car still shows.
    for (int i = 0; i < FRAME_W * FRAME_H; i++) {
        ref_frame[i] = (uint8_t)(((int)ref_frame[i] * 15 + (int)cur_frame[i]) >> 4);
    }

    delay(100); // ~10 fps
}
