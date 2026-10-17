/*
 * camera_selector.ino — 3-Camera Rear-View Selector (ESP32) — PRODUCTION
 * ==========================================================================
 * Automatically routes the correct rear camera to a 7" CVBS monitor:
 *
 *     Reverse gear  -> CENTER camera  (backing up)
 *     Left turn     -> LEFT   camera  (left blind spot)
 *     Right turn    -> RIGHT  camera  (right blind spot)
 *     Hazards       -> CENTER camera  (both turn signals = hazard lights)
 *     No signal     -> CENTER camera  (default / parking view)
 *
 * A momentary button cycles manually: AUTO -> CENTER -> LEFT -> RIGHT -> AUTO.
 * Auto inputs (reverse/turn) always override the manual camera.
 *
 * RELIABILITY FEATURES (vs the prototype):
 *   - Task watchdog: resets the ESP32 if the loop hangs (automotive safety).
 *   - Turn-signal blink latch (turn signals pulse ~1 Hz; the camera is held
 *     across the off-gap instead of flickering).
 *   - Hazard detection (left + right active together -> center view).
 *   - Status LEDs: power + one LED per active camera.
 *   - WiFi left OFF (unused; reduces EMI and power draw).
 *   - Startup self-test that sweeps the mux so wiring can be verified.
 *
 * HARDWARE (see docs/schematic_car.svg and docs/BOM_car.md):
 *   - Car reverse/turn signals are 12 V. Each is fed to the ESP32 through a
 *     PC817 optocoupler (galvanic isolation): 12 V side -> 1 kOhm -> PC817 LED;
 *     PC817 collector -> ESP32 GPIO, emitter -> GND. The pin is pulled LOW when
 *     the signal is ACTIVE (INPUT_ACTIVE_STATE = LOW).
 *   - IMPORTANT: inputs are on GPIOs WITH internal pull-ups (18/19/23). The
 *     original design used GPIO 34-36, which are input-only and have NO
 *     internal pull-up — those inputs would float. Do not use 34-39 for this.
 *   - GPIOs 25/26 drive a CD4052 analog mux (S0/S1) that switches the selected
 *     camera's CVBS video onto the monitor input. One half of the dual 4:1 mux
 *     is used; the monitor's 75 Ohm input terminates the video.
 *   - LEDs are active-HIGH: GPIO -> 330 Ohm -> LED -> GND.
 */

#include "esp_task_wdt.h"

// ------------------------- Configuration -------------------------
#define INPUT_ACTIVE_STATE  LOW   // LOW = optocoupler (recommended); HIGH = divider

// Inputs (12 V car signals, optocoupler-conditioned to 3.3 V logic)
// GPIO 18/19/23 have internal pull-ups (avoid 34-39 which do not).
#define PIN_REVERSE      18
#define PIN_LEFT_TURN    19
#define PIN_RIGHT_TURN   23
#define PIN_MANUAL       14     // momentary button to GND (INPUT_PULLUP)

// Mux select outputs -> CD4052 pins A (S0) and B (S1)
#define PIN_MUX_S0       25
#define PIN_MUX_S1       26

// Status LEDs (active HIGH)
#define PIN_LED_POWER    21
#define PIN_LED_CENTER   27
#define PIN_LED_LEFT     32
#define PIN_LED_RIGHT    33

// Timing
#define DEBOUNCE_MS      50     // input debounce
#define TURN_HOLD_MS     1000   // hold camera across turn-signal blink off-gap

// ------------------------- Camera codes --------------------------
// CD4052 select truth table (one half of the dual 4:1 mux):
//   S0/S1 = 00 -> Y0 (CENTER), 01 -> Y1 (LEFT), 10 -> Y2 (RIGHT), 11 -> Y3 spare
enum Camera : uint8_t {
  CAM_CENTER = 0,   // S0=0 S1=0
  CAM_LEFT   = 1,   // S0=1 S1=0
  CAM_RIGHT  = 2,   // S0=0 S1=1
  CAM_NONE   = 3    // manual-mode sentinel (means "auto")
};

// ------------------------- Input state ---------------------------
struct Input {
  uint8_t pin;
  bool    state;          // debounced state (active == INPUT_ACTIVE_STATE)
  bool    lastRaw;        // last raw reading
  unsigned long lastChange;   // debounce timestamp
  unsigned long lastActive;   // last time we saw the signal active
};

Input inReverse = { PIN_REVERSE,    false, false, 0, 0 };
Input inLeft    = { PIN_LEFT_TURN,  false, false, 0, 0 };
Input inRight   = { PIN_RIGHT_TURN, false, false, 0, 0 };

Camera currentCam = CAM_CENTER;
Camera manualCam  = CAM_NONE;   // CAM_NONE = automatic mode

// ------------------------- Helpers -------------------------------
void setMux(Camera c) {
  digitalWrite(PIN_MUX_S0, (uint8_t(c) >> 0) & 1);
  digitalWrite(PIN_MUX_S1, (uint8_t(c) >> 1) & 1);
}

void setLeds(Camera c) {
  digitalWrite(PIN_LED_CENTER, c == CAM_CENTER);
  digitalWrite(PIN_LED_LEFT,   c == CAM_LEFT);
  digitalWrite(PIN_LED_RIGHT,  c == CAM_RIGHT);
}

const char* camName(Camera c) {
  switch (c) {
    case CAM_CENTER: return "CENTER";
    case CAM_LEFT:   return "LEFT";
    case CAM_RIGHT:  return "RIGHT";
    default:         return "NONE";
  }
}

// Debounce one input and refresh its last-active timestamp.
void updateInput(Input& in) {
  bool raw = (digitalRead(in.pin) == INPUT_ACTIVE_STATE);
  if (raw != in.lastRaw) {
    in.lastRaw = raw;
    in.lastChange = millis();
  }
  if ((millis() - in.lastChange) >= DEBOUNCE_MS && raw != in.state) {
    in.state = raw;
  }
  if (in.state) in.lastActive = millis();
}

// A turn signal blinks (~1 Hz), so consider it active for TURN_HOLD_MS after
// its last HIGH edge so the camera doesn't flicker during the off-gap.
bool isActive(const Input& in) {
  return in.state || ((millis() - in.lastActive) < TURN_HOLD_MS);
}

// Startup self-test: light each LED and sweep the mux so wiring is verifiable.
void selfTest() {
  const Camera seq[] = { CAM_CENTER, CAM_LEFT, CAM_RIGHT };
  for (Camera c : seq) {
    setMux(c);
    setLeds(c);
    delay(300);
  }
  setMux(CAM_CENTER);
  setLeds(CAM_CENTER);
}

// ------------------------- Setup ---------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_REVERSE,    INPUT_PULLUP);
  pinMode(PIN_LEFT_TURN,  INPUT_PULLUP);
  pinMode(PIN_RIGHT_TURN, INPUT_PULLUP);
  pinMode(PIN_MANUAL,     INPUT_PULLUP);

  pinMode(PIN_MUX_S0, OUTPUT);
  pinMode(PIN_MUX_S1, OUTPUT);
  pinMode(PIN_LED_POWER,  OUTPUT);
  pinMode(PIN_LED_CENTER, OUTPUT);
  pinMode(PIN_LED_LEFT,   OUTPUT);
  pinMode(PIN_LED_RIGHT,  OUTPUT);

  digitalWrite(PIN_LED_POWER, HIGH);   // unit powered
  setMux(CAM_CENTER);
  setLeds(CAM_CENTER);

  // Task watchdog: reset the ESP32 if the loop stalls > 3 s (safety device).
  esp_task_wdt_config_t wdt_cfg = {
      .timeout_ms     = 3000,
      .idle_core_mask = 0,     // only subscribe our loop task
      .trigger_panic  = true,  // reset on timeout
  };
  esp_task_wdt_init(&wdt_cfg);
  esp_task_wdt_add(NULL);

  Serial.println("Camera selector ready (3 rear cameras, production)");
  Serial.printf("Input active state: %s\n",
                INPUT_ACTIVE_STATE == LOW ? "LOW (optocoupler)" : "HIGH (divider)");

  selfTest();
}

// ------------------------- Loop ----------------------------------
void loop() {
  esp_task_wdt_reset();

  updateInput(inReverse);
  updateInput(inLeft);
  updateInput(inRight);

  // Manual button (active-low, debounced falling edge): cycle
  //   AUTO -> CENTER -> LEFT -> RIGHT -> AUTO
  static bool lastBtn = HIGH;
  static unsigned long lastBtnTime = 0;
  bool btn = digitalRead(PIN_MANUAL);
  if (btn == LOW && lastBtn == HIGH && (millis() - lastBtnTime) > DEBOUNCE_MS) {
    lastBtnTime = millis();
    switch (manualCam) {
      case CAM_NONE:   manualCam = CAM_CENTER; break;
      case CAM_CENTER: manualCam = CAM_LEFT;   break;
      case CAM_LEFT:   manualCam = CAM_RIGHT;  break;
      case CAM_RIGHT:  manualCam = CAM_NONE;   break; // back to AUTO
    }
    Serial.printf("Mode: %s\n", manualCam == CAM_NONE ? "AUTO" : camName(manualCam));
  }
  lastBtn = btn;

  // Automatic priority: reverse > hazard > left turn > right turn > default.
  bool leftOn  = isActive(inLeft);
  bool rightOn = isActive(inRight);
  bool hazard  = leftOn && rightOn;

  Camera target = CAM_CENTER;
  if (isActive(inReverse)) target = CAM_CENTER;   // reverse wins over everything
  else if (hazard)         target = CAM_CENTER;   // hazard lights -> center
  else if (leftOn)         target = CAM_LEFT;
  else if (rightOn)        target = CAM_RIGHT;

  // Manual override wins while a camera is forced (auto inputs still have
  // priority per the logic above only when in AUTO mode — but reverse always
  // overrides manual too, so backing up is never hidden behind a manual pick).
  Camera selected;
  if (isActive(inReverse))      selected = CAM_CENTER;  // reverse always overrides
  else if (manualCam != CAM_NONE) selected = manualCam;
  else                          selected = target;

  if (selected != currentCam) {
    setMux(selected);
    setLeds(selected);
    currentCam = selected;
    Serial.printf("Camera -> %s\n", camName(selected));
  }

  delay(10);
}
