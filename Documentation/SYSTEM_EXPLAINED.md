# System explained

This document covers how the system works and, more importantly, **why** each
decision was taken. The design choices below are the ones worth defending in a
review.

## The problem

Passengers crowd around whichever door they happen to be standing next to, so
some cars are packed while others travel half empty. The information that would
fix this — which car has room — exists on the train but never reaches the
platform in time to be useful.

## The shape of the system

```
seat sensors ─┐
              ├─► ESP32 ─┬─► platform light strip (one LED per car)
simulated  ───┘          ├─► live web page (phone)
cars                     └─► (planned) platform screen
```

One physical car is instrumented. The other cars are simulated from the web
page. This is deliberate: the interesting engineering is in detecting a seat
reliably and presenting the result, not in repeating the same sensor twenty
times. Simulation lets the full platform experience be demonstrated with the
hardware that exists.

## Why ultrasonic distance sensing

A seat is either empty, and the sensor sees the seat surface at a known
distance, or occupied, and the sensor sees a person much closer. That is a
distance measurement, which an HC-SR04 does cheaply and without privacy
concerns.

A camera would also work and would be more precise, but it raises privacy
questions on public transport, needs far more processing, and sits outside what
the course supports. Pressure mats under the cushion would be the obvious
industrial choice, but they are not in the lab's parts catalogue.

## Why a seat does not change state immediately

Three mechanisms sit between a raw reading and a reported seat state, and each
exists because of a specific failure seen in testing.

**Smoothing.** Raw readings jitter, especially at a distance, because the
sensor's sound cone hits several surfaces and the echoes arrive mixed. Each new
reading is blended into a running average (`new = 0.7 × old + 0.3 × reading`),
which removes the jitter without adding noticeable lag. This is the exponential
moving average recommended in the course's own sensor notes.

**Two thresholds instead of one.** With a single threshold, a reading sitting
exactly on the boundary makes the seat flicker between states many times a
second. The system uses 35 cm to become taken and 45 cm to become free again, so
a reading has to move decisively before anything changes.

**A stay time.** A person walking past a seat is physically indistinguishable
from a person sitting down, for the first instant. The difference is duration. A
change is only committed after the new condition has held for two seconds, which
removes passers-by entirely.

## Why a sensor fault is its own state

If a sensor is unplugged or fails, it returns no echo. Treating that as "very
far away" would report the seat as free, and passengers would be guided toward a
car based on a seat that does not exist. A sensor silent for three seconds is
therefore reported as a fault — shown in blue on the strip and named on the page
— so a wrong answer is never presented as a right one.

## Why the board serves its own page

The ESP32 runs a small web server, so the status is visible from any phone on
the same network with no app to install and no cloud account. The cost is that
the page is only reachable on the local network, and that history is not stored
anywhere. Moving to a cloud database would fix both and is the obvious next step
if time allows.

## Why network loss does not stop anything

Sensing and the light strip run in the main loop and do not depend on the
network at all. The WiFi connection is retried in the background every five
seconds. So the platform lights, which are the part a passenger actually looks
at, keep working through a network outage, and the page returns on its own once
the network does. This was tested by switching the hotspot off and on again
mid-run.

## Known limits

- Soft surfaces absorb ultrasound, so a person in a thick coat returns a weaker
  echo than a hard object at the same distance.
- Two sensors close together can hear each other's echoes. They are deliberately
  pinged 60 ms apart to avoid this.
- A bag left on a seat reads as an occupied seat. Arguably correct, since the
  seat is unavailable, but worth stating.
- Thresholds are currently fixed in code. A calibration button that learns each
  seat's empty distance and stores it in flash is the planned fix.
