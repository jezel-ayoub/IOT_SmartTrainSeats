# Hardware unit tests

One sketch per component, plus a test that exercises the whole wiring at once.

Each sketch is standalone: it tests a single thing and prints its result to the
serial monitor at 115200 baud. The point is to be able to attribute a fault to a
specific component instead of guessing at it from the behaviour of the finished
system — a dead sensor and an unpowered sensor look identical from the outside.

| Sketch | What it proves |
|---|---|
| HW_Ultrasonic_Test | Each HC-SR04 returns a plausible distance that tracks a real object |
| HW_LED_Strip_Test | All three pixels light, in the right order and the right colours |
| HW_Wiring_Test | Both sensors and the strip work together on the final wiring |

## When to run which

**After changing any wiring** — run `HW_Wiring_Test` before loading the main
program. It covers all eleven connections in one pass, and it is faster to run
one test than to debug the full system.

**When one component misbehaves** — run that component's own test. It removes
every other variable, so whatever it reports is about that component alone.

Results are recorded in `HARDWARE_UNIT_TESTS_REPORT.md`.
