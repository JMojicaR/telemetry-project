# BOM — 3-Camera Rear-View Selector (Production, DIY kit)

CVBS analog build, no certification required (DIY kit). Prices are mid-range
estimates in MXN (MercadoLibre México / Amazon.com.mx / Steren, Sept 2026).
The hardening parts (fuse, reverse-polarity diode, TVS, optocouplers) are what
turn this from a bench prototype into something that survives a car.

## 1. Cameras & monitor

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 1 | CVBS/AHD dual-mode rear camera, 170°, IP68, night vision | 3 | $300 | $900 |
| 2 | 7" CVBS LCD monitor (2× AV input) | 1 | $700 | $700 |

Buy **3 identical** cameras and run them in **CVBS mode** (the AHD/CVBS switch
is on the camera's pigtail). Keeps the mux simple and leaves an AHD upgrade path.

## 2. Power (12 V automotive, hardened)

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 3 | Blade fuse holder + 3 A fuse | 1 | $50 | $50 |
| 4 | SS54 Schottky diode (5 A) — reverse-polarity protection | 1 | $12 | $12 |
| 5 | P6KE18A TVS diode — load-dump/transient clamp | 1 | $15 | $15 |
| 6 | LM2596 buck module, set to 5 V (powers the ESP32) | 1 | $60 | $60 |
| 7 | 470 µF / 25 V electrolytic + 0.1 µF ceramic (input filter) | 1 | $12 | $12 |
| 8 | 100 µF / 16 V electrolytic + 0.1 µF ceramic (5 V filter) | 1 | $10 | $10 |

Cameras and the monitor run directly on **12 V** (through the same fuse).

## 3. Signal conditioning (12 V → 3.3 V)

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 9 | PC817 optocoupler (galvanic isolation from car transients) | 3 | $10 | $30 |
| 10 | 1 kΩ resistor (opto LED current ~12 mA @ 12 V) | 3 | $1 | $3 |

Reverse, left-turn, and right-turn each get one optocoupler. The ESP32's
internal pull-up on GPIO 18/19/23 holds the pin HIGH; the opto pulls it LOW
when the 12 V signal is active. **Do not** move these to GPIO 34–39 (input-only,
no pull-up — they float).

## 4. Control & switching

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 11 | ESP32 DevKit V1 (WROOM-32) | 1 | $180 | $180 |
| 12 | CD4052 analog mux (DIP, dual 4:1) | 1 | $25 | $25 |
| 13 | 100 nF ceramic decoupling (ESP32 + mux) | 3 | $2 | $6 |
| 14 | 2N3904 NPN + 2 resistors — optional video buffer | 1 | $10 | $10 |

The CD4052 switches the CVBS video (one half of the dual 4:1). The monitor's
75 Ω input terminates the video. If the picture looks soft, add the 2N3904
emitter-follower buffer after the mux (see schematic).

## 5. Indicators & controls

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 15 | LED (power + 3× camera status) | 4 | $2 | $8 |
| 16 | 330 Ω resistor (LED current limit) | 4 | $1 | $4 |
| 17 | Momentary push button (manual cycle) | 1 | $10 | $10 |

## 6. Enclosure, connectors & wiring

| # | Part | Qty | Unit | Total |
|---|------|-----|------|-------|
| 18 | Enclosure box (ESP32 + PCB, ~10×6×4 cm) | 1 | $100 | $100 |
| 19 | RCA pigtails (3× camera in, 1× monitor out) | 4 | $15 | $60 |
| 20 | Automotive wire 18–20 AWG (power + signal harness) | 5 m | $15 | $75 |
| 21 | Heat shrink, solder, cable ties, mounting HW | 1 set | $80 | $80 |
| 22 | (Optional) custom PCB, small batch | 1 | $150 | $150 |

## Total

| Scope | Cost |
|-------|------|
| Cameras + monitor | $1,600 |
| Power (hardened) | $159 |
| Signal conditioning | $33 |
| Control & switching | $221 |
| Indicators & controls | $22 |
| Enclosure / connectors / wiring | $315 |
| **Subtotal (no PCB)** | **$2,350** |
| **With custom PCB** | **~$2,500** |

## What makes it a *real* product (vs the prototype)

- **Reverse-polarity diode + TVS + fuse** — survives miswiring and load-dump.
- **Optocoupler isolation** — the ESP32 never touches the noisy 12 V rail.
- **Fixed input pull-up bug** — inputs on GPIO 18/19/23 (not 34–36).
- **Task watchdog** — resets the ESP32 if it ever hangs (safety-critical).
- **Hazard + blink-latch logic** — no camera flicker on turn signals.
- **Status LEDs + self-test** — install/debug without a laptop.
- **Weatherproof cameras + enclosed controller** — car-ready.

*(Prices vary with USD/MXN and stock; re-check before ordering.)*
