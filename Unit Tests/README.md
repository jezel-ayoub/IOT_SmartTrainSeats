# Hardware Unit Tests

One standalone sketch per piece of hardware. Each one can be uploaded on its
own, prints `PASS` or `FAIL` to the Serial Monitor at 115200 baud, and tests
nothing but the part it is named after.

| Sketch | Hardware under test |
|---|---|
| `HW_Ultrasonic_Test/` | Both HC-SR04 ultrasonic distance sensors |
| `HW_LED_Strip_Test/` | WS2812 light strip (3 LEDs) |

Results are recorded in [HARDWARE_UNIT_TESTS_REPORT.md](HARDWARE_UNIT_TESTS_REPORT.md).

## How to run any of them

1. Open the `.ino` file in the Arduino IDE.
2. Tools → Board → **DOIT ESP32 DEVKIT V1**, then Tools → Port.
3. Upload. If the upload does not start, hold the **BOOT** button until the
   percentages appear.
4. Tools → Serial Monitor, speed **115200**.
5. Copy or screenshot the `PASS` / `FAIL` lines as evidence.
