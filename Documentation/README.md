# Smart Train Seat Guide

An IoT system that detects which train seats are free and tells waiting
passengers on the platform which car to board.

Technion, courses 236332 / 236333 — smart transportation.

## The idea

Passengers crowd into the nearest door and then walk the length of a train
looking for a seat. The information needed to avoid that already exists on the
train; it simply never reaches the platform. This project closes that gap.

An ultrasonic sensor above each seat reports whether it is occupied. A
controller aggregates the seats of each car, drives a colour-coded light per car
on the platform, and serves a live display showing the whole train and
recommending which car to board.

## How it works

Each HC-SR04 sensor measures the distance to the seat below it. A short
distance means someone is sitting there.

Raw ultrasonic readings are noisy, so a reading is not trusted on its own:

- **Smoothing** — an exponential moving average removes single-sample spikes.
- **Hysteresis** — two thresholds, one to become occupied and a higher one to
  become free, so a reading hovering at the boundary cannot oscillate.
- **Dwell time** — a change must persist for two seconds before it counts, so a
  passenger walking past does not register as sitting down.
- **Fault state** — a sensor that returns no echo for three seconds is reported
  as faulty rather than silently read as an empty seat.

## Car colours

| Colour | Meaning |
|---|---|
| Green | More than half the seats free |
| Yellow | Some free, but half or fewer |
| Red | Full |
| Blue | A sensor in that car has failed |

The same rule drives both the physical LEDs and the web display, from one
definition, so the two can never disagree.

## The train

Car 1 is the pilot car: two seats with real sensors, updating by themselves.
Cars 2 and 3 are simulated and respond to a seat being tapped on the display,
which allows a full three-car train to be demonstrated from one instrumented
car.

## The platform display

The controller runs a web server on the local network and serves a station
display: the train drawn from above, seats coloured live, the recommended car
highlighted, and a connection indicator that falls back to demo mode when the
page is opened away from the controller.

The page is served as three small responses — document, stylesheet, script —
rather than one large one. A single response of that size does not survive the
ESP32's socket buffer intact, and the symptom is a page that loads but never
runs its script.

## Repository layout

```
ESP32/seat_guide/         the main program
Unit Tests/               one sketch per component, plus the test report
Documentation/            wiring, physical model, versions, system explanation
```

## Hardware

- ESP32 DevKit V1 (DOIT)
- 2 x HC-SR04 ultrasonic sensors
- WS2812 LED strip, 3 pixels
- Breadboard and male-to-female jumpers

Wiring is in `Documentation/HARDWARE_WIRING.md`. Toolchain and library versions
are in `Documentation/VERSIONS.md`.

## Running it

1. Open `ESP32/seat_guide/seat_guide.ino` in the Arduino IDE.
2. Replace `YOUR_HOTSPOT_NAME` and `YOUR_HOTSPOT_PASSWORD` with your own
   network credentials.
3. Select the ESP32 Dev Module board and set the upload speed to 115200.
4. Upload, then open the serial monitor at 115200. It prints the address to
   open in a browser.

Before loading the main program onto new or changed wiring, run
`Unit Tests/HW_Wiring_Test` first — it confirms every connection in one pass.
