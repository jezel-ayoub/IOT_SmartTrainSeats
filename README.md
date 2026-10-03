# Smart Train Seat Guide

Ultrasonic sensors under train seats detect whether each seat is taken. A light
strip and a display on the platform tell waiting passengers which car has the
most free seats, so people spread along the train before the doors open.

Part of the IoT project course, Taub Faculty of Computer Science, Technion.

- **Project:** Smart transportation — train seat occupancy and platform guidance
- **Group:** ב2
- **Team:** מדהלה יובל, עבדו סלין, ג'יזל אבו איוב

## What it does

| | |
|---|---|
| Seat sensing | Two HC-SR04 sensors, one per seat, in the real car |
| False-alarm filtering | A seat counts as taken only after someone stays for 2 seconds, so passers-by are ignored |
| Fault handling | A sensor that stops responding is reported as a fault, never as a free seat |
| Platform lights | One LED per car: green (plenty of room), yellow (filling up), red (full), blue (sensor fault) |
| Live page | A web page served by the board shows every car and recommends the best one |
| Network resilience | If WiFi drops, sensing and lights keep working and the page returns by itself |
| Simulated cars | Cars 2 and 3 are driven from the page, so a whole train is demonstrated with one physical car |

## Hardware

| Qty | Component | Role |
|-----|-----------|------|
| 1 | ESP32 DevKit V1 (DOIT) | Reads the sensors, drives the lights, serves the page |
| 2 | HC-SR04 ultrasonic distance sensor | One per seat in the real car |
| 1 | WS2812 (NeoPixel) light strip, 3 LEDs | Platform status light per car |
| 2 | Breadboard | The ESP32 is wider than one breadboard |
| ~10 | Jumper wires (M-M and M-F) | Wiring |
| 1 | USB-C cable / phone hotspot | Power and network |

Pin assignments are in [Documentation/HARDWARE_WIRING.md](Documentation/HARDWARE_WIRING.md).

## Repository layout

- `ESP32/seat_guide/` — the main program
- `Unit Tests/` — one standalone test sketch per piece of hardware, plus the test report
- `Documentation/` — how the system works and why it was built this way

## Running it

1. Install the Arduino IDE and, in Boards Manager, the **esp32** package by Espressif.
2. In Library Manager, install **Adafruit NeoPixel**.
3. Open `ESP32/seat_guide/seat_guide.ino` and fill in `WIFI_NAME` and `WIFI_PASSWORD`.
4. Select board **DOIT ESP32 DEVKIT V1** and the serial port, then upload.
5. Open the Serial Monitor at 115200 baud; it prints the address of the page.
6. Open that address on a phone connected to the same network.

Real credentials are deliberately not committed. The file ships with placeholders.
