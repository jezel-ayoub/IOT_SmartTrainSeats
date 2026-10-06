# Hardware unit test report

Each test is a standalone sketch under `Unit Tests/`. They exist so that a
fault can be attributed to a specific component rather than guessed at from the
behaviour of the whole system.

| Test | What it proves | Result |
|---|---|---|
| HW_Ultrasonic_Test | Each HC-SR04 returns a plausible, changing distance | Pass |
| HW_LED_Strip_Test | All three WS2812 pixels light in the commanded colour | Pass |
| HW_Wiring_Test | Both sensors and the strip work together on the final wiring | Pass |

## HW_Wiring_Test

Added after the wiring was rebuilt. It is the acceptance test run before the
main program is loaded, and after any change to the physical wiring.

**Method.** The sketch drives both trigger lines and times both echo lines, and
sets the three strip pixels to green, amber and red.

**Pass criteria.**

1. Three LEDs light, in the commanded colours and order.
2. Both sensors report a distance in the 3–350 cm range.
3. Both readings change when a hand is moved in front of the sensor.

**Why all three matter.** A sensor with no power reports nothing at all, which
looks identical to a sensor that is broken. A sensor wired to the wrong pin
reports a constant value, which looks identical to an empty seat. Requiring the
reading to *change* separates a working sensor from a merely quiet one.

**Result.** Both sensors tracked a hand smoothly; all three pixels lit. Passed.

## Notes on earlier failures

**Flash communication failure during upload.** Uploads failed with "Failed to
communicate with the flash chip" whenever a jumper was attached to D12. GPIO12
is sampled at boot to select the flash voltage. Resolved by moving sensor B's
echo line to D35 and setting the upload speed to 115200.

**Both sensors silent after re-wiring.** Traced to jumpers seated in a
detached breadboard strip that was not connected to the controller. Covered in
`HARDWARE_WIRING.md`.
