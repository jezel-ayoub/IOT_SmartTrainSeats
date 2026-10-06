# Physical demonstration model

A single open-topped train car with two real, sensor-equipped seats, and a
separate station board carrying the three status LEDs.

Every dimension below is chosen so that the ultrasonic sensors see an
unambiguous difference between an empty seat and an occupied one. In a scale
model all distances shrink, and a structure chosen without thought gives nearly
the same reading in both cases.

## Geometry

| Measurement | Value |
|---|---|
| Car floor | 40 x 24 cm |
| Side walls | 10 cm high |
| Aisle width | 6 cm |
| Seat cushion | 8 x 7 cm, 4 cm high |
| Seat back | 8 x 8 cm |
| Sensor beam height above floor | 30 cm |
| Station board | 25 x 11 cm |
| LED spacing on the board | 7.5 cm |

Each sensor is mounted on the beam, centred over its seat and aimed straight
down.

## The two readings this produces

| Seat state | Distance measured |
|---|---|
| Empty | about 26 cm |
| Occupied by a 20 cm doll | about 12 cm |

A 14 cm gap, which leaves generous margin on both sides of the decision points.

## Thresholds for the model

The defaults in the main sketch are set for a full-size seat. For the model:

```cpp
const float TAKEN_BELOW = 18;
const float FREE_ABOVE  = 22;
```

After the model is glued, both readings should be measured again on the serial
monitor and these numbers adjusted if the gap has moved by more than a
centimetre or two.

## Wiring inside the model

The sensors sit on the beam; the controller is hidden in a box behind the
station board. That distance exceeds a single jumper, so jumpers are chained:
the male pin of a second jumper into the female socket of the first. At most two
are chained, each joint is taped, and the total run is kept under about one
metre. Cables are gathered along the beam and dropped behind the model, tied so
nothing is visible from the front.

Row and pin assignments are in `HARDWARE_WIRING.md`.

## Build order

The sensors are not glued down until step 6. Measure first, then commit.

| # | Step | Time |
|---|---|---|
| 1 | Cut all parts | 60 min |
| 2 | Glue the car: floor, two sides, one end | 30 min |
| 3 | Build and place the seats | 30 min |
| 4 | Raise the beam on its two uprights | 25 min |
| 5 | Paint, and let it dry | 45 min + drying |
| 6 | Mount the sensors with tape, measure both readings, then glue | 45 min |
| 7 | Update the thresholds and upload | 15 min |
| 8 | Build the station board and fit the LED strip | 30 min |
| 9 | Hide the electronics, tie the cables | 30 min |
| 10 | Rehearse the demonstration | 30 min |

About six hours in total, best spread over two days so paint and glue can dry.

A full Hebrew version of this plan, with measured drawings, a shopping list and
a cutting list, is kept alongside the project for use during construction.
