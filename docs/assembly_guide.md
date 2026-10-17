# Assembly & Installation Guide — DIY kit

How to build the 3-camera rear-view selector and install it in a car. Read the
**schematic** (`docs/schematic_car.svg`) and **BOM** (`docs/BOM_car.md`) first.

## 1. Build the controller board

1. **Power rail.** Wire the 12 V input in this order: fuse → SS54 Schottky
   (band toward the buck) → LM2596 set to 5 V. Put the P6KE18A TVS *across*
   the 12 V input (cathode to +12 V, anode to GND). Add the 470 µF cap on the
   input and 100 µF on the 5 V output. The buck's 5 V feeds **ESP32 VIN**; the
   12 V rail feeds the cameras and monitor directly.
2. **Signal inputs.** For each of reverse / left / right: 12 V → 1 kΩ → PC817
   LED anode; LED cathode → GND. PC817 collector → ESP32 GPIO (18/19/23),
   emitter → GND. The ESP32's internal pull-up reads HIGH when idle and LOW
   when the signal is active.
3. **Video switching.** Route the three camera CVBS wires into CD4052 Y0/Y1/Y2.
   Drive S0/S1 from ESP32 GPIO 25/26. Take the common output to the monitor
   AV input (add the 2N3904 buffer if the picture is soft).
4. **Indicators & button.** Four LEDs via 330 Ω (power + 3 camera status).
   Momentary button from GPIO 14 to GND.
5. **Flash** `firmware/camera_selector/camera_selector.ino` to the ESP32.

## 2. Bench test (before the car)

1. Power with 12 V — the **power LED** lights and the mux **self-tests**
   (sweeps CENTER→LEFT→RIGHT, lighting each LED).
2. Touch +12 V to the reverse input — monitor should show the **center**
   camera, center LED on.
3. Pulse +12 V on the left-turn input (simulate blinking) — the **left** camera
   should hold steady across the blink gap (no flicker).
4. Press the button to cycle AUTO → CENTER → LEFT → RIGHT → AUTO.
5. Confirm reverse always overrides the manual camera.

## 3. Install in the car

1. **Mount** the three cameras at the rear (center above the plate, left/right
   on the bumper or tail edges) with the 170° field covering the blind spots.
   Run the camera cables forward to the controller.
2. **Tap signals** at the tail-light harness: reverse light wire, left-turn
   wire, right-turn wire. Splice the three optocoupler inputs there — this is
   what makes the switching automatic.
3. **Power** the controller from an **ignition-switched** 12 V source (fused),
   so it's off when the car is off.
4. **Monitor** on the dash; feed the mux output to its AV1 input.

## 4. Safety notes

- The fuse must be the **first** thing on the 12 V feed, as close to the tap as
  possible.
- Do **not** run unfused wires through the firewall — use a grommet.
- Double-check the reverse/turn taps with a multimeter before soldering
  (turn signals pulse ~1 Hz; reverse is steady).
- This is a driving aid, **not** a replacement for looking back — verify the
  camera view before relying on it while reversing.
