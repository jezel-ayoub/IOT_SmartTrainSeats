// ---------------------------------------------------------------
// HW unit test: HC-SR04 ultrasonic distance sensors
//
// Checks that both sensors respond and report a plausible distance.
// Prints PASS or FAIL once per second for each sensor.
//
// How to run:
//   1. Tools -> Board -> DOIT ESP32 DEVKIT V1, and select the port
//   2. Upload (hold BOOT if the upload does not start)
//   3. Tools -> Serial Monitor, speed 115200
//   4. Move your hand toward and away from each sensor
//
// Expected result: both sensors print PASS, and the distance follows
// your hand. Hold a flat object at a measured 20 cm to check accuracy.
// ---------------------------------------------------------------

const int TRIG_A = 27;
const int ECHO_A = 26;
const int TRIG_B = 13;
const int ECHO_B = 12;

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.0343 / 2.0;
}

void report(const char* name, float cm) {
  Serial.print("HW TEST: ");
  Serial.print(name);
  Serial.print(" | distance: ");
  if (cm <= 0) {
    Serial.println("no echo | Result: FAIL - check wiring and power");
    return;
  }
  Serial.print(cm);
  Serial.print(" cm | Result: ");
  if (cm > 2 && cm < 400) Serial.println("PASS - reading is in the sensor's range");
  else Serial.println("FAIL - reading outside the sensor's range");
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_A, OUTPUT); pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT); pinMode(ECHO_B, INPUT);
  Serial.println("HW TEST: HC-SR04 ultrasonic distance sensors");
}

void loop() {
  report("Seat A sensor", readDistance(TRIG_A, ECHO_A));
  delay(60);
  report("Seat B sensor", readDistance(TRIG_B, ECHO_B));
  delay(940);
}
