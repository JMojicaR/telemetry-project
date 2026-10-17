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

---

## K210 node — implemented (follow-up)

`firmware/k210_node/main.py` is a **Sipeed Maix (K210)** camera node that runs
**YOLOv2 (20-class VOC)** on the KPU — it detects cars/buses/persons/bicycles
directly (not just "motion"), and estimates distance with a **class-aware**
model: `distance = focal_px × real_width[class] / box_width_px`.

| Object | Real width (m) |
|--------|----------------|
| car | 1.8 |
| bus | 2.5 |
| person | 0.5 |
| bicycle | 0.6 |
| motorbike | 0.8 |
| train | 2.9 |

### Integration with the display unit

The K210 replaces one ESP32-CAM (keep the other, or use two K210s). It sends
the closest object over **UART** as `class,distance_m,cx,width\n`. Wire the
K210's **UART1 TX → S3 SuperMini RX**, then read it in the display unit:

```cpp
// add to display_unit.ino setup():  Serial1.begin(115200, SERIAL_8N1, RX_PIN, -1);
// add to loop():
while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    int cls, cx, w; float d;
    if (sscanf(line.c_str(), "%d,%f,%d,%d", &cls, &d, &cx, &w) == 4) {
        k210Dist = d; lastK210 = millis();   // reuse the color dashboard
    }
}
```

### K210 BOM addition

| Component | Qty | Unit | Total |
|-----------|-----|------|-------|
| Sipeed Maix Bit / Dock (K210) + OV2640 | 1–2 | $450 | $450–900 |
| MicroSD (for the .kmodel) | 1 | $60 | $60 |

### Notes

- Flash MaixPy with **kflash**, copy the Sipeed 20-class `yolo.kmodel` to
  `/sd/yolo.kmodel` (or rename), and **calibrate `FOCAL_LENGTH_PX`**.
- MaixPy is MicroPython — `main.py` is flashed to the K210, not compiled with
  the Arduino toolchain, so it can't be build-tested here (no K210 hardware).
