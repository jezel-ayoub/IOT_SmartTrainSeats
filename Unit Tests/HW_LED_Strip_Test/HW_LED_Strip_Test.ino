// ---------------------------------------------------------------
// HW unit test: WS2812 (NeoPixel) light strip, 3 LEDs
//
// Cycles all three LEDs through red, green and blue, which are the
// three colours the platform display uses for car status.
//
// Requires the "Adafruit NeoPixel" library (Library Manager).
//
// How to run:
//   1. Tools -> Board -> DOIT ESP32 DEVKIT V1, and select the port
//   2. Upload (hold BOOT if the upload does not start)
//   3. Tools -> Serial Monitor, speed 115200
//   4. Watch the strip
//
// Expected result: all three LEDs show the colour named in the
// Serial Monitor, one second apart.
// ---------------------------------------------------------------

#include <Adafruit_NeoPixel.h>

const int LED_PIN   = 14;
const int LED_COUNT = 3;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(40);   // dim, to keep the current draw low
  strip.show();
  Serial.println("HW TEST: WS2812 light strip on pin 14");
  Serial.println("PASS - strip initialised");
}

void loop() {
  strip.fill(strip.Color(255, 0, 0)); strip.show();
  Serial.println("RED   - expect all three LEDs red");
  delay(1000);

  strip.fill(strip.Color(0, 255, 0)); strip.show();
  Serial.println("GREEN - expect all three LEDs green");
  delay(1000);

  strip.fill(strip.Color(0, 0, 255)); strip.show();
  Serial.println("BLUE  - expect all three LEDs blue");
  delay(1000);
}
