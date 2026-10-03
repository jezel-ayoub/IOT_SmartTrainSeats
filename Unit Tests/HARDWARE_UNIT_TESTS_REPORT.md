# Hardware Unit Tests Report

Project: Smart Train Seat Guide
Group: _fill in your group number_

## Tests

| # | Hardware | Test method | Result | Test code |
|---|---|---|---|---|
| 1 | ESP32 DevKit V1 | Uploaded a blink sketch; the on-board LED blinked at 1 Hz and the upload log reported `Hash of data verified` | PASS | Arduino IDE built-in Blink |
| 2 | HC-SR04 sensor, seat A | Sketch pings the sensor and prints the distance; the reading follows a hand moved toward and away from the sensor | PASS | `HW_Ultrasonic_Test/` |
| 3 | HC-SR04 sensor, seat B | Same method, on the second sensor | PASS | `HW_Ultrasonic_Test/` |
| 4 | HC-SR04 accuracy | A flat object placed at a ruler-measured 20 cm; the sensor reported 20 cm | PASS | `HW_Ultrasonic_Test/` |
| 5 | WS2812 light strip | Sketch cycles all three LEDs through red, green and blue | PASS | `HW_LED_Strip_Test/` |
| 6 | WiFi on the ESP32 | The board joined a phone hotspot and printed its address; the page opened from a phone | PASS | `ESP32/seat_guide/` |
| 7 | Recovery from network loss | The hotspot was switched off for 20 s and back on; sensing and lights continued, and the page returned without a reset | PASS | `ESP32/seat_guide/` |

## Notes and observations

- **Readings beyond roughly one metre fluctuate.** The HC-SR04 emits a wide
  cone, so distant echoes come back from several objects at once. Close
  readings, which is what seat detection uses, are stable.
- **Soft surfaces absorb ultrasound.** Fabric returns a weaker echo than a hard
  surface, which is one reason the sensor is mounted close to the seat.
- **Supply voltage.** The HC-SR04 is specified for 5 V while the ESP32 works at
  3.3 V, and the sensor's Echo pin is currently wired straight to the board.
  This works, but whether a voltage divider should be added on the Echo line is
  an open question for the lab engineer.

## Evidence

- Serial Monitor output from each sketch above
- A short video of both seats changing between FREE and TAKEN, and of the
  platform lights changing colour
