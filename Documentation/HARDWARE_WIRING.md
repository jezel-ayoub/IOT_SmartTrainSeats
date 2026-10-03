# Hardware wiring

The ESP32 DevKit V1 is wider than a single breadboard, so it straddles two:
one row of pins in each. All connections below are on the pin row that carries
`VIN`, `GND`, `D13`, `D12`, `D14`.

## Seat sensors (HC-SR04)

| Sensor pin | Seat A | Seat B |
|---|---|---|
| VCC | 3V3 | 3V3 |
| Trig | D27 | D13 |
| Echo | D26 | D12 |
| GND | GND | GND |

The Trig/Echo pin assignments were confirmed by a pin-scan sketch that pulsed
each candidate pin in turn and reported which pair produced a valid echo.

## Platform light strip (WS2812 / NeoPixel, 3 LEDs)

| Strip wire | Strip pad | ESP32 pin |
|---|---|---|
| red | +5V | VIN |
| black | GND | GND |
| yellow | Din | D14 |

`VIN` is the 5 V rail coming from the USB connector. The course tutorial wires
the strip's data line to D12; this project uses **D14** instead, because D12 is
already taken by the Echo line of the seat B sensor.

Brightness is set to 40 of 255 in software. Three LEDs at that level draw far
less than the USB supply provides, so no separate power supply is needed.

## Power notes

- Everything is powered from the USB connection to a laptop or a USB power bank.
- The HC-SR04 is specified for 5 V while the ESP32 is a 3.3 V device. The
  sensors currently run from 3V3 and their Echo pins connect directly to the
  board. This works reliably in testing. Whether to move the sensors to VIN and
  add a resistor divider on each Echo line is an open question for the lab
  engineer.
