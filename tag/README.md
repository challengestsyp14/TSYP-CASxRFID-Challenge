# Tag firmware (ESP32 DevKit V1)

`main_tag_v19/main_tag_v19.ino` is the firmware: events (tap, drop, impact, upside_down, tamper, temp_breach), flash log, Wi-Fi upload, the OLED screen with knob and push button, and the handling profile (FRAGILE yes/no, TEMP MODE cold / room / medication; an alert stays on the screen with the red LED on until the button is pressed). v19 also sends the profile to the backend with each upload, so the dashboard shows it (compiles; v18 without this ran on the box, 15 Nov 2026).

The sketch is in its own folder because the Arduino IDE needs the folder name to match the .ino name. Wiring, serial commands and the change history are in the comment at the top of the sketch.

## Before flashing

1. Edit `WIFI_SSID` and `WIFI_PASS` (a 2.4 GHz hotspot, the same one as the laptop that runs the backend), `SERVER_IP` (that laptop) and `BOX_ID`. Do not commit real Wi-Fi credentials.
2. Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB SPIFFS)" (the model does not fit in the default one).
3. Add the model library with Sketch > Include Library > Add .ZIP Library (the zip in this folder).

## Build used

- Board: ESP32 Dev Module (DevKit V1), esp32 core 3.1.3.
- Libraries: Adafruit GFX Library 1.13.0, Adafruit SH110X 2.1.15, Adafruit BusIO 1.17.4, Adafruit MPU6050 2.2.9, Adafruit Unified Sensor 1.1.15, MFRC522 1.4.12, DHT sensor library 1.4.7, parcel-tag_inferencing 1.0.6.
- Screen chip: 1.3" modules have the SH110X chip. For a 0.96" module (SSD1306) install Adafruit SSD1306 2.5.17 instead and set `OLED_IS_SSD1306` to 1 in the sketch.

## Model library

`ei-parcel-tag-arduino-1.0.6-impulse-1.zip` is the Edge Impulse deployment the tag runs (project `parcel-tag`, ID 1152202, generated 12 Nov 2026). It is the only model export in this repository.

Test results for this deployed int8 model: overall accuracy 96.06 %, ROC AUC 1.00, weighted precision / recall / F1 0.97, on a dataset of 239 labelled recordings (186 train, 53 test).

| Class | Accuracy (int8) | F1 | Uncertain |
|---|---|---|---|
| carried | 93.9 % | 0.97 | 4.8 % |
| impact | 97.1 % | 0.98 | 1.0 % |
| still | 97.9 % | 0.99 | 1.5 % |
| upside_down | 93.4 % | 0.94 | 6.6 % |

Earlier model versions are described in `ml/README.md` for history; their figures describe those versions, not this one. The sketch only checks that the library has four labels.
