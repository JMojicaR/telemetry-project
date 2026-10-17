# ============================================================================
# Bicycle rear-view camera node — Sipeed Maix (Kendryte K210), MaixPy v1
# ----------------------------------------------------------------------------
# The #1 upgrade over the ESP32-CAM node: runs YOLOv2 (20-class VOC) on the
# KPU to actually detect CARS / BUSES / PERSONS / BICYCLES, then estimates the
# distance with a class-aware monocular model:
#
#   distance_m = FOCAL_LENGTH_PX * real_width[class] / bounding_box_width_px
#
# Because the object CLASS is known, we use the correct real width (a person
# ~0.5 m, a car ~1.8 m) — far more accurate than the ESP32-CAM's "assume a car"
# motion-blob. Outputs the closest object over UART and draws boxes on the LCD.
#
# Run on a Maix Bit / Maix Dock with the OV2640 camera. Flash with kflash;
# copy the Sipeed 20-class model to /sd/yolo.kmodel (rename if needed).
#
# NOTE: MaixPy is MicroPython — this is flashed to the K210, not compiled here.
# ============================================================================

import sensor
import image
import lcd
import time
import KPU as kpu
from machine import UART
from fpioa_manager import fm

# ---- UART output (to the ESP32-S3 display unit) ----
# Maix Bit pins 10/11 → UART1; wire to the S3 SuperMini's spare RX.
fm.register(10, fm.fpioa.UART1_TX, force=True)
fm.register(11, fm.fpioa.UART1_RX, force=True)
uart = UART(UART.UART1, 115200, timeout=1000, read_buf_len=4096)

# ---- Camera ----
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)       # 320x240
sensor.skip_frames(time=2000)
lcd.init(freq=15000000)
lcd.rotation(2)                          # mirror for a rear-facing mount

# ---- Load the 20-class YOLOv2 model (Sipeed "m.kmodel" / "yolo.kmodel") ----
task = kpu.load("/sd/yolo.kmodel")
ANCHORS = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658,
           5.155437, 6.92275, 6.718375, 9.01025)
kpu.init_yolo2(task, 0.5, 0.3, 5, ANCHORS)   # threshold, nms, max boxes, anchors

# VOC class names + real-world widths (meters) for distance.
LABELS = ["aeroplane","bicycle","bird","boat","bottle","bus","car","cat","chair",
          "cow","diningtable","dog","horse","motorbike","person","pottedplant",
          "sheep","sofa","train","tvmonitor"]
REAL_WIDTH = {
    1: 0.6,    # bicycle  (handlebar width)
    2: 0.5,    # bird
    5: 2.5,    # bus
    6: 1.8,    # car
    13: 0.8,   # motorbike
    14: 0.5,   # person
    18: 2.9,   # train
}
FOCAL_LENGTH_PX = 277.0   # OV2640 @ QVGA(320px), ~60° HFOV — CALIBRATE

def distance_m(cls, box_w_px):
    rw = REAL_WIDTH.get(cls, 1.8)         # fall back to car width
    return FOCAL_LENGTH_PX * rw / float(box_w_px)

while True:
    img = sensor.snapshot()
    objects = kpu.run_yolo2(task, img)

    closest = None   # (class_id, distance_m, center_x, width)
    if objects:
        for o in objects:
            cls = o.classid()
            if cls in REAL_WIDTH:
                x, y, w, h = o.rect()
                d = distance_m(cls, w)
                cx = x + w // 2
                if closest is None or d < closest[1]:
                    closest = (cls, d, cx, w)
                img.draw_rectangle(x, y, w, h, color=(255, 0, 0), thickness=2)
                img.draw_string(x, max(0, y - 12),
                                "%s %.1fm" % (LABELS[cls], d),
                                color=(255, 255, 255), scale=1)

    if closest:
        cls, d, cx, w = closest
        # Text protocol to the display unit: "class,distance_m,cx,width\n"
        uart.write("%d,%.1f,%d,%d\n" % (cls, d, cx, w))

    lcd.display(img)
    time.sleep_ms(20)   # detection dominates the loop time (~15-20 fps)
