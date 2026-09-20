# Component Research & Pricing

Research snapshot dated **September 2026**. Prices are mid-range estimates in
Mexican pesos (MXN) from MercadoLibre México, Amazon.com.mx, and Steren.
Exchange rate assumption: ~1 USD = 18 MXN.

## 1. Rear cameras

Universal car rear cameras with **170° wide angle, IP68 waterproof, night vision**
and dual **AHD/CVBS** output are the standard choice (AHD = analog HD 720p/1080p,
CVBS = standard definition 480i). Both are 12 V powered.

| Type | Typical price (MXN) | Notes |
|------|---------------------|-------|
| CVBS 170° (basic) | $150–300 | Cheapest, easiest to switch with a mux |
| AHD 720p/1080p 170° | $250–450 | Better quality, harder to switch (needs DVR or video-grade switch) |

**Recommendation:** buy 3 identical **AHD/CVBS dual-mode** cameras (~$300 each)
and run them in **CVBS mode** for the base build so the CD4052 can switch them.
They remain upgradeable to AHD later.

## 2. 7" monitor

7-inch LCD monitors for rear-view use are ubiquitous, 12 V powered, usually with
2× AV inputs (AV1/AV2) and sometimes a trigger wire that auto-switches to AV2 on
reverse.

| Type | Typical price (MXN) | Notes |
|------|---------------------|-------|
| 7" LCD, 2× AV in | $500–900 | Standard choice; the 2nd AV input can carry the switched feed |
| 7" AHD monitor | $800–1,200 | Needed only for the AHD/DVR upgrade path |

**Recommendation:** a 7" LCD with **2 AV inputs** (~$700). Feed the mux output
into AV1; leave AV2 free (or wire it directly to the center camera as a
fail-safe full-time reverse view).

## 3. Microcontroller

| Option | Price (MXN) | Notes |
|--------|-------------|-------|
| ESP32 DevKit (WROOM-32) | $150–220 | Recommended — plenty of GPIO, WiFi for future telemetry |
| ESP32-C3 SuperMini | $80–130 | Cheaper, single-core RISC-V; enough for this task |
| Raspberry Pi Zero 2 W | $400–700 | Overkill; only if you need recording/object detection |

**Recommendation:** ESP32 DevKit (~$180). It handles the 3 signal inputs, the
manual button, and the 2 mux select lines, with GPIO to spare.

> Why not drive the cameras directly? The ESP32 cannot decode analog camera
> video; its job is **switching**, not processing. The analog multiplexer does
> the actual video routing.

## 4. Video switch (analog multiplexer)

| Option | Price (MXN) | Notes |
|--------|-------------|-------|
| CD4052 (dual 4:1) | $15–40 | Through-hole, easy; on-resistance ~100 Ω (fine for CVBS prototype) |
| MAX4544 (video SPDT) | $30–60 | Video-grade, low distortion; use 2× for 3 inputs |
| MAX4310 (8:1 video mux) | $50–120 | Best quality, SMD, single chip |
| 4-ch AHD DVR | $1,500–2,500 | The "all 3 at once" upgrade (Option B), not a switch |

**Recommendation:** CD4052 for the prototype (DIP, breadboard-friendly). If the
video looks soft or noisy, swap in a **MAX4310** or use the AHD DVR path.

## 5. Signal conditioning (12 V → 3.3 V)

Car reverse and turn signals are **12–14.4 V**. The ESP32 GPIO is 3.3 V.

| Option | Cost | Pros / Cons |
|--------|------|-------------|
| **Optocoupler PC817** (recommended) | ~$10 each | Galvanic isolation from car transients; clean logic level |
| Resistor divider (22 kΩ + 4.7 kΩ) | ~$2 | Cheapest, but no isolation; shares car ground |

**Recommendation:** optocouplers (PC817) — automotive electrical noise and
load-dump transients can destroy a directly-connected ESP32. Cheap insurance.

## 6. Power

| Option | Price (MXN) | Notes |
|--------|-------------|-------|
| DC-DC buck 12 V→5 V (e.g. LM2596) | $40–80 | Powers the ESP32 from the car's 12 V |
| Inline blade fuse holder + 3 A fuse | $30–60 | Mandatory on the 12 V feed |
| TVS diode (e.g. P6KE18A) | $10–20 | Suppresses transients on the 12 V rail |

Cameras and the monitor run directly on 12 V (fused).

## 7. Sources checked

- MercadoLibre México — rear cameras, 7" monitors, mobile AHD DVRs
- Amazon.com.mx — rear camera listings, LED/AV monitors
- Steren — motion/LED accessories (no purpose-built multi-camera car switcher found)
- RS / distributor listings — CD4052/HEF4053 analog mux pricing

*(Prices vary with USD/MXN and stock; re-check before ordering.)*
