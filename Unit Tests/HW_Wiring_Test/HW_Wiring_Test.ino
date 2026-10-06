// ---------------------------------------------------------------
// HARDWARE UNIT TEST - full wiring check
//
// Purpose: verify, in one run, that both HC-SR04 sensors and the
// WS2812 LED strip are wired correctly, before loading the main
// program. Used as the acceptance test after any re-wiring.
//
// Pass criteria:
//   1. Three LEDs light up: green, yellow, red.
//   2. Both sensors report a plausible distance (3 - 350 cm) and
//      the value changes when a hand is moved in front of them.
//
// Wiring under test:
//   Sensor A: Trig = D27, Echo = D26
//   Sensor B: Trig = D13, Echo = D35   (D35 is input-only)
//   LED strip: DIN = D14
//   Power for sensors and strip: VIN (5 V) and GND
// ---------------------------------------------------------------

#include <Adafruit_NeoPixel.h>

const int TRIG_A  = 27;
const int ECHO_A  = 26;
const int TRIG_B  = 13;
const int ECHO_B  = 35;
const int LED_PIN = 14;

Adafruit_NeoPixel strip(3, LED_PIN, NEO_GRB + NEO_KHZ800);

// Sends one ultrasonic ping and returns the distance in cm, or 0 on timeout.
float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(TRIG_A, OUTPUT);
  pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT);
  pinMode(ECHO_B, INPUT);

  strip.begin();
  strip.setBrightness(40);
  strip.setPixelColor(0, strip.Color(0, 255, 0));      // green
  strip.setPixelColor(1, strip.Color(255, 110, 0));    // yellow
  strip.setPixelColor(2, strip.Color(255, 0, 0));      // red
  strip.show();

  Serial.println();
  Serial.println("=== WIRING TEST ===");
  Serial.println("Expect 3 lights: green, yellow, red.");
  Serial.println("Wave a hand in front of each sensor.");
  Serial.println();
}

void loop() {
  float a = readDistance(TRIG_A, ECHO_A);
  delay(60);
  float b = readDistance(TRIG_B, ECHO_B);
  delay(60);

  Serial.print("Sensor A: ");
  if (a <= 0) Serial.print("-- no answer --");
  else { Serial.print(a); Serial.print(" cm"); }

  Serial.print("      Sensor B: ");
  if (b <= 0) Serial.println("-- no answer --");
  else { Serial.print(b); Serial.println(" cm"); }

  delay(300);
}
