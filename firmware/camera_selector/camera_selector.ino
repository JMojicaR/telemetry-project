/*
 * camera_selector.ino — 3-Camera Rear-View Selector (ESP32)
 * ============================================================
 * Automatically routes the correct rear camera to a 7" monitor:
 *
 *     Reverse gear  -> CENTER camera  (backing up)
 *     Left turn     -> LEFT   camera  (left blind spot)
 *     Right turn    -> RIGHT  camera  (right blind spot)
 *     No signal     -> CENTER camera  (default / parking view)
 *
 * A momentary button cycles manually:  AUTO -> CENTER -> LEFT -> RIGHT -> AUTO.
 *
 * HARDWARE (see PLAN.md / README.md for the full wiring):
 *   - Car signals are 12 V. Feed them to the ESP32 through an optocoupler
 *     (PC817, recommended — galvanic isolation) or a resistor divider.
 *   - The ESP32 GPIOs drive a CD4052 analog multiplexer (2 select lines)
 *     that switches the selected camera's CVBS video onto the monitor input.
 *
 * SIGNAL CONVENTION:
 *   Optocoupler wiring pulls the pin LOW when the 12 V signal is ACTIVE.
 *   Set INPUT_ACTIVE_STATE to HIGH if you use a non-inverting resistor divider.
 */

// ------------------------- Configuration -------------------------
#define INPUT_ACTIVE_STATE  LOW   // LOW = optocoupler (recommended); HIGH = divider

// Inputs (12 V car signals, conditioned to 3.3 V logic)
#define PIN_REVERSE      34
#define PIN_LEFT_TURN    35
#define PIN_RIGHT_TURN   36
#define PIN_MANUAL       14     // momentary button to GND (INPUT_PULLUP)

// Mux select outputs -> CD4052 pins A (S0) and B (S1)
#define PIN_MUX_S0       25
#define PIN_MUX_S1       26

// Timing
#define DEBOUNCE_MS      50     // input debounce
#define TURN_HOLD_MS     800    // keep camera during turn-signal blink off-gap

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

Input inReverse = { PIN_REVERSE,   false, false, 0, 0 };
Input inLeft    = { PIN_LEFT_TURN, false, false, 0, 0 };
Input inRight   = { PIN_RIGHT_TURN,false, false, 0, 0 };

Camera currentCam = CAM_CENTER;
Camera manualCam  = CAM_NONE;   // CAM_NONE = automatic mode

// ------------------------- Helpers -------------------------------
void setMux(Camera c) {
  digitalWrite(PIN_MUX_S0, (uint8_t(c) >> 0) & 1);
  digitalWrite(PIN_MUX_S1, (uint8_t(c) >> 1) & 1);
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

// ------------------------- Setup ---------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_REVERSE, INPUT_PULLUP);
  pinMode(PIN_LEFT_TURN, INPUT_PULLUP);
  pinMode(PIN_RIGHT_TURN, INPUT_PULLUP);
  pinMode(PIN_MANUAL, INPUT_PULLUP);

  pinMode(PIN_MUX_S0, OUTPUT);
  pinMode(PIN_MUX_S1, OUTPUT);
  setMux(CAM_CENTER);

  Serial.println("Camera selector ready (3 rear cameras)");
  Serial.printf("Input active state: %s\n", INPUT_ACTIVE_STATE == LOW ? "LOW" : "HIGH");
}

// ------------------------- Loop ----------------------------------
void loop() {
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

  // Automatic priority: reverse > left turn > right turn > default(center)
  Camera target = CAM_CENTER;
  if (isActive(inReverse))      target = CAM_CENTER;
  else if (isActive(inLeft))    target = CAM_LEFT;
  else if (isActive(inRight))   target = CAM_RIGHT;

  // Manual override wins while a camera is forced
  Camera selected = (manualCam != CAM_NONE) ? manualCam : target;

  if (selected != currentCam) {
    setMux(selected);
    currentCam = selected;
    Serial.printf("Camera -> %s\n", camName(selected));
  }

  delay(10);
}
