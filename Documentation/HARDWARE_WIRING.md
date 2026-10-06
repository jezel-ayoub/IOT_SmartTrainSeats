# Hardware wiring

Smart Train Seat Guide — ESP32 DevKit V1 (DOIT), 2 x HC-SR04, WS2812 strip (3 LEDs).

## How the board is mounted

The ESP32 is seated directly in the breadboard so that every one of its pins
sits in a known numbered row. The board is oriented with the USB connector at
the row-30 end, so the pins fall in descending row order.

The five holes `a`–`e` of a row are internally connected. The ESP32's own pin
occupies one of them, which leaves the rest of that row free — a jumper pushed
into any free hole of the row is therefore connected to that pin. This is the
whole of the wiring scheme: **put each wire in the same row as its pin.**

| ESP32 pin | Breadboard row | Used for |
|---|---|---|
| VIN  | 30 | +5 V rail for both sensors and the LED strip |
| GND  | 29 | common ground |
| D13  | 28 | sensor B — Trig |
| D14  | 26 | LED strip — DIN |
| D27  | 25 | sensor A — Trig |
| D26  | 24 | sensor A — Echo |
| D35  | 20 | sensor B — Echo (input-only pin) |

## Connection list

Every connection uses a male-to-female jumper: the female end grips the
component's header pin, the male end is pushed into the breadboard.

### Sensor A (seat A)

| Sensor pin | Breadboard row |
|---|---|
| VCC  | 30 |
| GND  | 29 |
| Trig | 25 |
| Echo | 24 |

### Sensor B (seat B)

| Sensor pin | Breadboard row |
|---|---|
| VCC  | 30 |
| GND  | 29 |
| Trig | 28 |
| Echo | 20 |

### LED strip (WS2812, 3 pixels)

| Strip wire | Signal | Breadboard row |
|---|---|---|
| red    | 5V  | 30 |
| black  | GND | 29 |
| yellow | DIN | 26 |

Rows 30 and 29 each carry three wires plus the ESP32's own pin, in four
different holes of the same row.

## Design notes

**Why VIN and not 3V3.** The HC-SR04 is specified for 5 V. Driven from the
3.3 V rail it returns short and unstable readings. VIN carries the 5 V supplied
over USB, which is what both the sensors and the WS2812 strip need.

**Why D35 for sensor B's echo.** GPIO34, 35, 36 and 39 are input-only on the
ESP32, which suits an echo line and removes any risk of driving them as outputs.

**Why not D12.** GPIO12 is a strapping pin: its level is sampled at boot to
select the flash voltage. A jumper holding it high prevents the board from
flashing at all ("Failed to communicate with the flash chip"). It was the
original sensor B echo line and was moved to D35 for this reason.

**LED strip direction.** WS2812 strips are directional. The arrows printed on
the strip must point away from the connected end; wired to the other end the
strip simply stays dark.

**Extending a jumper.** The male pin of a second jumper is pushed into the
female socket of the first. At most two are chained, each joint is taped so it
cannot work loose, and the total run is kept under about one metre. This is how
the sensors reach the beam of the physical model while the controller stays in
its box — see `PHYSICAL_MODEL.md`.

## A failure worth recording

During assembly both sensors went silent the moment they were moved from being
seated directly in the breadboard onto jumper wires. The sensors were fine: the
breadboard was made of separable strips that had come apart, and the jumpers had
been pushed into a strip that was not connected to the ESP32 at all, so neither
power nor signal reached them.

It had worked beforehand only because the sensors, seated directly, happened to
land in the same rows as the controller's pins — the connection was being made
by accident rather than by design.

The fix was to seat the ESP32 itself in the breadboard, giving every pin a known
row, and then place each wire in the row belonging to its pin. The lesson is
recorded here because the symptom (a dead sensor) pointed at entirely the wrong
component.

## Verification

`Unit Tests/HW_Wiring_Test/HW_Wiring_Test.ino` exercises every connection in
this document in a single run. Results are in `HARDWARE_UNIT_TESTS_REPORT.md`.
