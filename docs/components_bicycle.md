# Bicycle option — Components & Suggested Improvements

## Bill of materials (approx, MercadoLibre / Amazon MX, Sept 2026)

| # | Component | Qty | Unit | Total |
|---|-----------|-----|------|-------|
| 1 | ESP32-CAM (AI-Thinker, OV2640) | 2 | $180 | $360 |
| 2 | ESP32-S3 SuperMini | 1 | $150 | $150 |
| 3 | ILI9341 2.4" SPI TFT (240×320) | 1 | $150 | $150 |
| 4 | Power bank 5 V (≥ 10,000 mAh) | 1 | $150 | $150 |
| 5 | Camera mounts + weatherproof housings | 2 | $100 | $200 |
| 6 | USB cables, dupont, heatshrink, misc | 1 set | $100 | $100 |
| | **Subtotal** | | | **$1,110** |
| | Contingency (~12%) | | | $140 |
| | **Total** | | | **~$1,250** |

Both the ESP32-CAM and the S3 SuperMini run on 5 V, so a single power bank
feeds everything (the S3's 3.3 V rail powers the TFT logic).

## Suggested improvements

The base build is intentionally minimal. These upgrades address its real
weaknesses (CV accuracy, distance reliability, weather, power, safety):

1. **Better CV board — the #1 upgrade.** The ESP32-CAM can only do simple
   motion blobs. For real "car/person behind" detection with reliable distance:
   - **Sipeed Maix (Kendryte K210)** — has a hardware KPU that runs YOLO/MobileNet
     at ~30 fps; ~$400–600 MXN. Drop-in better CV.
   - **ESP32-S3-EYE** — more RAM/CPU than the ESP32-CAM, plus an on-board LCD.
   - **Raspberry Pi Zero 2 W + camera** — full OpenCV / YOLO if you want real
     object detection (heavier, more power).
2. **True depth instead of monocular guessing** — a **VL53L1X ToF laser** (0–4 m)
   or an **ultrasonic** module as a redundant near-field sensor, or **stereo**
   (two synchronized cameras → disparity depth).
3. **Audio warning** — a **buzzer** (or bone-conduction headset) is safer than a
   screen while riding; the current design is screen-only per your choice.
4. **Weatherproofing (essential)** — IP65/67 enclosures for both cameras and a
   sealed display case; bike = rain, mud, vibration.
5. **Vibration damping** — soft mounts for the cameras; hard-mounting a rolling
   camera causes blur and false motion.
6. **Range** — the ESP32-CAM's PCB antenna is weak; the module has an **IPEX
   connector** option for an external antenna.
7. **Power upgrades** — a **2× 18650 pack + BMS** (or a dynamo-hub buck
   converter) for longer rides than a power bank.
8. **Dashcam recording** — the ESP32-CAM has an **SD card** slot; log JPEG
   frames as evidence.
9. **GPS / speed input** — a GPS module to make the warning threshold
   speed-aware (warn earlier at higher speed).
10. **Better display** — a **round IPS (GC9A01)** or **OLED** for daylight
    readability, or a 2.8" touch for settings.

### Priority recommendation

For a *safety* device, upgrade in this order: **(1) K210 for real detection →
(3) buzzer → (4) weatherproofing → (2) ToF near-field sensor.** The monocular
ESP32-CAM build is a great proof-of-concept, but vehicles are safety-critical —
detecting "a car" (not just "motion") is what actually prevents accidents.
