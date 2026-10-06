// ---------------------------------------------------------------
// HARDWARE UNIT TEST - WS2812 LED strip
//
// Purpose: prove that all three pixels light, show the commanded
// colour, and are addressed in the expected order, independently of
// the sensors.
//
// Pass criteria:
//   1. Each pixel lights on its own, in order, first to last.
//   2. All three then show green, amber and red together.
//   3. The colours shown match the colours commanded. A strip wired
//      as RGB rather than GRB will swap red and green here.
//
// Pins:
//   DIN = D14,  power from VIN (5 V) and GND
//
// Note: the strip is directional. The arrows printed on it must point
// away from the connected end, or nothing lights at all.
// ---------------------------------------------------------------

#include <Adafruit_NeoPixel.h>

const int LED_PIN   = 14;
const int LED_COUNT = 3;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(500);

  strip.begin();
  strip.setBrightness(40);
  strip.show();

  Serial.println();
  Serial.println("=== LED STRIP TEST ===");
  Serial.println("Each pixel lights in turn, then all three together.");
  Serial.println();
}

void loop() {
  // One pixel at a time, so the addressing order can be checked.
  for (int i = 0; i < LED_COUNT; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    Serial.print("Pixel ");
    Serial.print(i);
    Serial.println(" should now be white.");
    delay(800);
  }

  // The three status colours used by the finished system.
  strip.setPixelColor(0, strip.Color(0, 255, 0));      // green
  strip.setPixelColor(1, strip.Color(255, 110, 0));    // amber
  strip.setPixelColor(2, strip.Color(255, 0, 0));      // red
  strip.show();
  Serial.println("All three: green, amber, red.");
  delay(2000);
}
