# Telemetry Project — 3-Camera Rear-View System

A car rear-view system with **3 cameras** mounted at the back of the vehicle,
an **ESP32 microcontroller** that auto-switches between them, and a **7-inch
monitor**. Designed to eliminate rear blind spots and make reversing/lane
changes safer.

---

## What it does

| Trigger | Camera shown |
|---------|--------------|
| Reverse gear | CENTER (backing up) |
| Left turn signal | LEFT (left blind spot) |
| Right turn signal | RIGHT (right blind spot) |
| No signal | CENTER (default) |
| Manual button | cycles CENTER → LEFT → RIGHT → AUTO |

The ESP32 reads the car's reverse and turn-signal wires (12 V), decides which
camera to show, and drives an analog multiplexer that routes that camera's video
to the monitor.

```
 [LEFT]  ─┐                    ┌────────────┐
 [CENTER]─┼─► CD4052 video mux ┼──► 7" monitor
 [RIGHT] ─┘        ▲           └────────────┘
                   │ S0,S1
 reverse/turn ──► ESP32 ──► auto camera selection
```

---

## Budget (≤ 5,000 MXN)

| Component | Qty | Unit | Total |
|-----------|-----|------|-------|
| Rear camera CVBS/AHD 170° IP68 | 3 | $300 | $900 |
| 7" LCD monitor (2× AV in) | 1 | $700 | $700 |
| ESP32 DevKit | 1 | $180 | $180 |
| CD4052 analog mux + socket | 1 | $30 | $30 |
| Optocouplers PC817 + resistors | 1 set | $60 | $60 |
| DC-DC buck 12 V → 5 V | 1 | $80 | $80 |
| RCA/BNC cables + adapters | 1 set | $150 | $150 |
| Enclosure + fuse + misc | 1 set | $180 | $180 |
| **Subtotal** | | | **$2,280** |
| Contingency (~15%) | | | $340 |
| **Total** | | | **~$2,620** |

*(Premium upgrade — 4-ch AHD DVR + 1080p cameras for a simultaneous quad view:
~$4,450 MXN. See PLAN.md.)*

---

## Repository layout

```
telemetry-project/
├── README.md                     ← this file
├── PLAN.md                       ← full plan, budget, milestones, risks
├── firmware/
│   └── camera_selector/
│       └── camera_selector.ino   ← ESP32 base firmware
└── docs/
    └── components.md             ← component research & pricing references
```

---

## Wiring (summary)

**Video path (CVBS):**
- Each camera's video out (RCA/BNC center pin) → CD4052 `Y0`, `Y1`, `Y2`
- CD4052 output (`Z`) → monitor AV input
- All video grounds tied together

**ESP32 control:**
- `GPIO25` → CD4052 `A` (S0)
- `GPIO26` → CD4052 `B` (S1)
- `GPIO34` ← reverse signal (via optocoupler)
- `GPIO35` ← left turn (via optocoupler)
- `GPIO36` ← right turn (via optocoupler)
- `GPIO14` ← manual button (to GND, internal pull-up)

**Signal conditioning (IMPORTANT — 12 V car signals):**
Each 12 V signal goes through an **optocoupler (PC817)** — a resistor (≈1 kΩ)
limits the LED current, and the transistor side pulls the ESP32 pin LOW when the
signal is active. This provides galvanic isolation from the noisy car electrical
system.

**Power:**
- Cameras and monitor run directly on 12 V.
- The ESP32 runs on a DC-DC buck converter (12 V → 5 V, fed to the `5V`/`VIN`
  pin), with a **3 A inline fuse** on the 12 V feed.

---

## Firmware — build & flash

```bash
# Install the ESP32 board package (once)
arduino-cli core install esp32:esp32

# Compile
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/camera_selector

# Flash (adjust the port)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 firmware/camera_selector

# Monitor
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Key firmware settings are at the top of `camera_selector.ino`:

```cpp
#define INPUT_ACTIVE_STATE  LOW    // LOW for optocoupler, HIGH for divider
#define PIN_REVERSE      34
#define PIN_LEFT_TURN    35
#define PIN_RIGHT_TURN   36
#define PIN_MANUAL       14
#define PIN_MUX_S0       25
#define PIN_MUX_S1       26
#define TURN_HOLD_MS     800    // keep camera during turn-signal blink
```

---

## Safety notes

- **Disconnect the battery** before tapping into the vehicle wiring.
- Use a **fuse (3 A)** on the 12 V feed, and never bypass it.
- The optocouplers isolate the ESP32 from the car's 12 V — do not wire 12 V
  directly to any GPIO.
- Secure cables with strain relief; IP68 cameras for water resistance.
- This is a driver aid, **not** a substitute for mirrors or looking — verify the
  view before reversing.

---

## References

- Full plan, architecture decision, milestones and risks: [PLAN.md](PLAN.md)
- Component research and pricing: [docs/components.md](docs/components.md)
- Base firmware: [firmware/camera_selector/camera_selector.ino](firmware/camera_selector/camera_selector.ino)
