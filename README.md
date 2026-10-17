# Smart Train Seat Guide

An IoT system that detects which train seats are free and tells waiting passengers
which car to board — before the doors open.

Technion, courses 236332 / 236333 · Group ב2 · October 2026

---

## How it works

An ultrasonic distance sensor is mounted on the ceiling above each seat. It sends a
sound pulse, measures how long the echo takes to return, and converts that to a
distance. An empty seat reads far; an occupied seat reads near, because the pulse
hits the passenger on the way down.

One ESP32 reads every sensor, decides free or taken, and produces two outputs: a
coloured LED on the platform, and a live web page it serves itself.

The system stores exactly one number per seat — a distance in centimetres. There is
no camera and no identification of any kind.

| Reading | Meaning |
| --- | --- |
| 13 cm | seat empty |
| 6 cm | someone sitting |
| below 8.5 cm | decide TAKEN |
| above 10.5 cm | decide FREE |

Between the two thresholds the system holds its previous answer. Any change must
persist for 2 seconds before it is recorded. Three seconds with no echo at all marks
the sensor as faulty — the system reports that it does not know, rather than
reporting "free".

---

## Repository layout

| Path | Contents |
| --- | --- |
| `ESP32/seat_guide/` | The main program. Everything that runs on the controller. |
| `Unit Tests/Sensor_Check/` | Diagnostic sketch: sensors only, no network, no decision logic. Use this for calibration. |
| `Documentation/` | Model dimensions, user manual, programmer manual. |
| `3D Model/` | A 3D model of the physical prototype, plus rendered views. |

---

## Hardware

| Component | Qty |
| --- | --- |
| ESP32 DevKit V1 (DOIT) | 1 |
| HC-SR04 ultrasonic sensor | 2 |
| WS2812B LED strip | 3 LEDs |
| Breadboard, jumper wires | — |

Pin assignment:

| Pin | Connected to | Direction |
| --- | --- | --- |
| 27 | TRIG, seat A | output |
| 26 | ECHO, seat A | input |
| 13 | TRIG, seat B | output |
| 35 | ECHO, seat B | input only |
| 14 | LED strip data | output |

Pin 35 is deliberate: on this board it is input-only, which makes it impossible for
the program to drive it by mistake. The sensors need 5 V — they are not reliable at
3.3 V.

---

## Building and uploading

1. Install the Arduino IDE (v2 or later).
2. Add ESP32 board support (Espressif board manager URL), then install the `esp32`
   package.
3. Install the **Adafruit NeoPixel** library.
4. Open `ESP32/seat_guide/seat_guide.ino`.
5. **Set your own network details** at the top of the file:

```cpp
const char* WIFI_NAME     = "YOUR_HOTSPOT_NAME";
const char* WIFI_PASSWORD = "YOUR_HOTSPOT_PASSWORD";
```

6. Board: `DOIT ESP32 DEVKIT V1`. Upload speed: 921600.
7. Upload, then open the Serial Monitor at **115200**. A line beginning `PASS`
   carries the IP address — open it in a browser.

The network must be 2.4 GHz. The ESP32 cannot see a 5 GHz network, and that is the
most common reason for "everything looks fine but it won't connect".

---

## Calibration

The thresholds in this repository match our model only. Any other seat, or any other
ceiling height, needs measuring again.

1. Upload `Sensor_Check.ino` and open the Serial Monitor.
2. Record the reading with the seat **empty** — call it `E`.
3. Record the reading with someone **sitting** — call it `O`.
4. Compute:

```
G = E - O
TAKEN_BELOW = (E + O) / 2  -  G / 7
FREE_ABOVE  = (E + O) / 2  +  G / 7
```

5. Put the results in `TAKEN_BELOW[]` and `FREE_ABOVE[]`, one entry per seat.

**The check that must pass:** `E` must be greater than `FREE_ABOVE`. If the empty
reading falls between the two thresholds, that seat can switch to TAKEN but can never
return — it looks exactly like a broken sensor, and it is not.

Each seat gets its own pair of numbers. Two sensors on the same ceiling do not give
the same empty reading, and a shared threshold is what caused that bug for us.

---

## Documentation

| File | For |
| --- | --- |
| `Documentation/MODEL_MEASUREMENTS_HE.pdf` | Every dimension needed to rebuild the physical model (Hebrew) |
| `Documentation/USER_MANUAL_HE.pdf` | Operating the system: startup, reading the light, troubleshooting (Hebrew) |
| `Documentation/PROGRAMMER_MANUAL_HE.pdf` | Code structure, algorithm, interface, how to extend it (Hebrew) |

---

## Security note

The copy of `seat_guide.ino` in this repository contains placeholder network
credentials. Replace them locally before uploading to the board, and do not commit
real credentials back to the repository.

---

## Limitations

- The sensor reports the nearest object in its cone and cannot tell a bag from a
  passenger.
- A very small child may not cross the threshold.
- Tested on two seats. Interference between many adjacent ultrasonic sensors in a
  real carriage has not been measured.
- The savings figures in the report come from a simulation using published models,
  not from field measurement.
