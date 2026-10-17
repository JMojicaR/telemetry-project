# Plan — Bicycle Rear-View with Distance (2× ESP32-CAM + ESP32-S3)

> **Goal:** a bicycle rear-view system with **two ESP32-CAM** cameras and a
> **2.4" TFT** driven by an **ESP32-S3 SuperMini**, which **estimates the
> distance to objects behind** using computer vision and warns the rider
> on-screen.

## 1. Decisions (confirmed)

| Question | Decision |
|----------|----------|
| Cameras | Both **rear** — one wide-angle (approaching cars) + one close-range (blind spot) |
| Display | **2.4" SPI TFT** (ILI9341, 240×320), driven by the S3 SuperMini |
| Warning | **On-screen distance + color** (no buzzer) |
| Power | **Power bank** (5 V USB) |

## 2. Architecture

```
 [ESP32-CAM #0] ──(CV: motion → distance)──┐
    wide-angle rear                        │ ESP-NOW (broadcast)
 [ESP32-CAM #1] ──(CV: motion → distance)──┼──► ESP32-S3 SuperMini ──► ILI9341 2.4"
    close-range rear                       │        (receiver)       color-coded dashboard
                                           ┘
```

- Each **ESP32-CAM** captures grayscale frames, runs **background subtraction**,
  finds the moving object's bounding box, and computes distance with a
  **monocular model** — then broadcasts the result over **ESP-NOW**.
- The **ESP32-S3 SuperMini** receives both streams and draws a two-panel
  dashboard: **"Vista amplia"** and **"Rango corto"**, colored
  green (>5 m) / yellow (2–5 m) / red (<2 m) / gray (no signal).

## 3. Computer vision — distance model

```
distance_m = FOCAL_LENGTH_PX × REAL_WIDTH_M / object_width_px

  FOCAL_LENGTH_PX = 125   (OV2640 @ 160 px, ~65° HFOV — CALIBRATE per camera)
  REAL_WIDTH_M    = 1.8   (assumed width of a car)
```

- Frame differencing (`|cur − ref| > 25`) → motion mask → bounding box.
- A slow **leaky background** (α ≈ 1/16) adapts to the road without erasing a
  fast-approaching car.
- If the moving area exceeds `MIN_AREA_PX`, the box width → distance.

**Honest limits:**
- This is **monocular** — it assumes the object is ~1.8 m wide (a car). A
  person, motorbike, or truck will read off-scale.
- **Works best stopped / slow** (traffic light, filtering). At speed the whole
  scene shifts and floods the motion mask. Robust *moving* detection needs a
  real detector (see `components_bicycle.md` → upgrades).
- Focal length must be **calibrated** per camera (print a 1.8 m target at a
  known distance and adjust `FOCAL_LENGTH_PX`).

## 4. Firmware

| Sketch | Board | Role |
|--------|-------|------|
| `firmware/cam_node/cam_node.ino` | ESP32-CAM (×2) | capture → CV → distance → ESP-NOW (set `CAM_ID` 0/1) |
| `firmware/display_unit/display_unit.ino` | ESP32-S3 SuperMini | ESP-NOW receive → ILI9341 dashboard |

Both compile (esp32 core 3.3.10). The ESP-NOW message struct must match between
the two sketches; nodes are fixed on **channel 1** (broadcast).

## 5. BOM (approx, MXN)

See `docs/components_bicycle.md` for the full table. Total ≈ **$1,250 MXN**
(2× ESP32-CAM, S3 SuperMini, ILI9341, power bank, mounts, wiring).

## 6. Milestones

1. Bench: 1× ESP32-CAM → S3 → TFT, verify distance + color works.
2. Add the second camera (wide vs close) and the two-panel dashboard.
3. Calibrate `FOCAL_LENGTH_PX` for both cameras.
4. Mount on the bike (weatherproof), test in traffic (stopped first).
5. Iterate `MOTION_THRESHOLD` / `MIN_AREA_PX` for real conditions.
