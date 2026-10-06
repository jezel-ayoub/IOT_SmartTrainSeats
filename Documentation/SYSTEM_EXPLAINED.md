# How the system works

A walk through the whole system, from a sound pulse leaving a sensor to a
coloured light on the platform.

## The problem

A passenger standing on a platform cannot see which car has seats. So people
board at the nearest door and then walk the length of the train, which slows
boarding, crowds some cars while others run half empty, and makes the dwell time
at each station longer than it needs to be.

The information needed to solve this already exists on the train. It simply
never reaches the platform. This project carries it across.

## The chain, end to end

1. An ultrasonic sensor above each seat measures the distance to the seat.
2. The controller turns that distance into a seat state, with filtering.
3. Seat states are aggregated into a state for the whole car.
4. The car state drives a coloured light on the platform and a live web display.

## Step 1: measuring

An HC-SR04 emits a short burst of sound above human hearing and times how long
the echo takes to return. Sound travels at roughly 343 metres per second, so:

```
distance = duration x 0.0343 / 2
```

The division by two is because the sound travels to the object and back.

An empty seat gives a long reading — the floor or the cushion. An occupied one
gives a short reading, because a person's body is much closer to the sensor.

## Step 2: from distance to seat state

A raw distance cannot be used directly. Three different things go wrong, and
each needs its own answer.

**Noise.** Ultrasonic readings jitter by a few centimetres, and occasionally
produce a wild value when an echo bounces off something unexpected. The answer
is an exponential moving average: each new reading is blended with the running
value rather than replacing it, so a single bad sample moves the result only
slightly.

```cpp
smooth = 0.7 * smooth + 0.3 * newReading
```

**Boundary oscillation.** If a single threshold decided between free and taken,
a reading sitting exactly on it would flip back and forth many times a second.
The answer is hysteresis: two thresholds instead of one. A seat becomes taken
below the lower threshold and free again only above the higher one. Between them
nothing changes, so the state is stable.

**Transient events.** Someone walking past the sensor, or reaching across a
seat, briefly produces exactly the reading a seated passenger produces. The
answer is a dwell time: a change must persist for two seconds before it is
accepted. Walking past does not last two seconds; sitting down does.

**A fourth case: the sensor itself.** If a sensor is disconnected or broken it
returns no echo at all. Treated naively that reads as a very large distance,
which means an empty seat — exactly the wrong answer, because the display would
confidently send passengers to a car it knows nothing about. So a sensor that
returns no echo for three seconds is marked faulty, and the car is shown in a
separate colour that means "no data" rather than "free".

## Step 3: from seats to a car state

```
more than half the seats free  ->  green
some free, but half or fewer   ->  yellow
none free                      ->  red
any sensor faulty              ->  blue
```

The threshold is "more than half" rather than "half or more" deliberately: a car
that is exactly half full is filling up, not roomy, and a passenger told it is
roomy would be misled.

This rule is written once and used by both the LED strip and the web page, so
the light on the platform and the display beside it can never disagree.

## Step 4: showing it

**On the platform.** A WS2812 strip with one pixel per car. A passenger reads it
at a glance, from a distance, without stopping.

**On the display.** The controller runs a web server on the local network and
serves a station board: the train drawn from above, each seat coloured live, the
recommended car highlighted, and the number of free seats per car. It refreshes
every second.

The page is served as three separate small responses — document, stylesheet,
script — rather than one large one. A single response of that size does not
survive the ESP32's socket buffer intact, and the symptom is a page that appears
to load but whose script never runs.

## The pilot car

Instrumenting a whole train would need dozens of sensors. Car 1 is the pilot
car: two seats with real sensors. Cars 2 and 3 are simulated, and a seat in them
can be tapped on the display to seat or remove a passenger.

This is a deliberate demonstration strategy, not a shortcut. It shows the real
sensing path end to end on genuine hardware, while still demonstrating the
aggregation, the recommendation and the full three-car display that a complete
installation would produce.

## Failure behaviour

The system is built so that losing a part degrades it rather than stopping it.

- **Wi-Fi drops** — sensing and the platform lights keep working. The controller
  retries the connection in the background every ten seconds.
- **A sensor fails** — that car turns blue. The other cars are unaffected, and
  the display says a sensor has failed rather than quietly guessing.
- **The page is opened away from the controller** — it falls back to demo mode
  and says so, instead of showing stale data as though it were live.
