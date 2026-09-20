# Plan — 3-Camera Rear-View System (7" screen, ESP32)

> **Goal:** build a system of **3 rear-facing cameras** on a car, connected to a
> **microcontroller** that drives a **7-inch monitor**, on a budget of
> **≤ 5,000 MXN** (~$278 USD @ 18 MXN/USD).

---

## 1. Requirements

| # | Requirement |
|---|-------------|
| R1 | 3 cameras mounted at the rear of the car (center, left blind spot, right blind spot) |
| R2 | A microcontroller selects / switches the cameras |
| R3 | A 7" screen shows the (selected) camera feed |
| R4 | Powered from the vehicle 12 V electrical system |
| R5 | Total bill of materials ≤ 5,000 MXN |
| R6 | Automatic switching on reverse gear and turn signals (the main value) |

---

## 2. Architecture decision

Two viable approaches were researched. We recommend **Option A** (microcontroller
+ analog switch) because it matches the requirement of a *microcontroller* in the
loop, is the cheapest, and is the easiest to build/repair.

### Option A — Microcontroller + analog switch (RECOMMENDED)
```
 [LEFT cam] ──┐
 [CENTER cam]─┤── CD4052 analog mux ── CVBS ──► 7" monitor
 [RIGHT cam]──┘        ▲
                       │ S0,S1 (GPIO)
 [reverse/turn signals] ──► ESP32 ──► selects camera
```
- 3 × CVBS/AHD cameras → analog multiplexer → single monitor input.
- ESP32 reads reverse + left/right turn signals and drives the mux select lines.
- **Shows one camera at a time**, auto-switched (the natural UX for blind spots).
- Cost: ~$2,600 MXN — well within budget.

### Option B — Mobile AHD DVR (upgrade: all 3 at once)
- 3 × AHD 1080p cameras → 4-channel **mobile AHD DVR** (quad/split view) → monitor.
- The DVR has trigger inputs that auto-switch to the corresponding camera
  full-screen on reverse / left / right signals (verified: these DVRs exist on
  MercadoLibre).
- The ESP32 becomes optional (or controls the DVR over RS-485).
- Cost: ~$4,400 MXN — still within budget, but more "black box" and less DIY.

**Decision:** build **Option A** first. Add Option B later if a simultaneous
quad-view is wanted.

---

## 3. Component selection (researched Sept 2026, MercadoLibre / Amazon MX / Steren)

### 3.1 Recommended build (Option A)

| # | Component | Qty | Unit (MXN) | Total | Notes |
|---|-----------|-----|-----------|-------|-------|
| 1 | Rear camera CVBS/AHD, 170°, IP68, night vision | 3 | $300 | $900 | AHD/CVBS universal, 12 V, RCA/BNC |
| 2 | 7" LCD monitor, 2× AV input | 1 | $700 | $700 | Standard rear-view monitor, 12 V |
| 3 | ESP32 DevKit (ESP32-WROOM-32) | 1 | $180 | $180 | Or ESP32-C3 ($100) to save |
| 4 | CD4052 dual 4:1 analog mux + DIP socket | 1 | $30 | $30 | Video switch prototype |
| 5 | Optocouplers PC817 + resistors (signal conditioning) | 1 set | $60 | $60 | Isolate 12 V signals → 3.3 V |
| 6 | DC-DC buck converter 12 V → 5 V (or 3.3 V) | 1 | $80 | $80 | Powers the ESP32 |
| 7 | RCA/BNC cables + adapters + wire | 1 set | $150 | $150 | Camera runs + video cable |
| 8 | Enclosure, inline fuse, terminals, misc | 1 set | $180 | $180 | Waterproof box, 3 A fuse |
| | **Subtotal** | | | **$2,280** | |
| | Contingency (~15%) | | | $340 | |
| | **Total (base build)** | | | **~$2,620** | |

### 3.2 Premium upgrade (Option B — quad view)

| Component | Qty | Unit | Total |
|-----------|-----|------|-------|
| AHD 1080p rear camera | 3 | $350 | $1,050 |
| 4-channel mobile AHD DVR (trigger auto-switch) | 1 | $2,000 | $2,000 |
| 7" AHD monitor | 1 | $900 | $900 |
| Cables, power, misc | 1 set | $500 | $500 |
| **Total (premium)** | | | **~$4,450** |

---

## 4. Budget summary

| Build | Total (MXN) | Within 5,000? |
|-------|-------------|---------------|
| Base (Option A) | ~$2,620 | ✅ (48% headroom) |
| Premium (Option B) | ~$4,450 | ✅ (11% headroom) |

Both fit. The base build leaves budget for better cameras, a video-grade mux, or
an enclosure upgrade.

---

## 5. Implementation plan (milestones)

| Phase | Task | Deliverable | Est. effort |
|-------|------|-------------|-------------|
| 1 | Prototype on bench: 3 cameras + CD4052 + ESP32 + monitor | Video switches on demand | 1–2 days |
| 2 | Signal conditioning: reverse/turn 12 V → 3.3 V (optocouplers) | Clean, isolated inputs | 0.5 day |
| 3 | Firmware: auto-switch + manual button (see `firmware/`) | Working selector | 0.5 day |
| 4 | Vehicle install: mount cameras, run cables, tap signals | On-car prototype | 1 day |
| 5 | Hardening: enclosure, fuse, strain relief, waterproofing | Road-ready unit | 1 day |
| 6 | (Optional) Upgrade to AHD + mobile DVR for quad view | Premium build | 1–2 days |

---

## 6. Risks & mitigations

| Risk | Mitigation |
|------|-----------|
| Car electrical noise / transients damage ESP32 | Optocoupler isolation + inline fuse + TVS diode |
| CD4052 on-resistance degrades video | Use short 75 Ω cables; upgrade to video-grade mux (MAX4544/MAX4310) if noisy |
| Turn signals blink (~1 Hz) | Firmware `TURN_HOLD_MS` keeps the camera held during the off-gap |
| 12 V vs 14.4 V (alternator) over-volts logic | Optocoupler input rated for 12–14.4 V; buck converter handles the ESP32 rail |
| Water ingress on rear cameras | Use IP68 cameras; seal cable entry points |

---

## 7. Next steps

1. Order the base-build BOM (§3.1).
2. Bench-test the video switch + firmware before touching the car.
3. Install cameras and tap reverse/turn signals (use a multimeter first).
4. Iterate firmware timing (`TURN_HOLD_MS`, `DEBOUNCE_MS`) in the real car.

Full component research and pricing references are in `README.md` and
`docs/components.md`.
