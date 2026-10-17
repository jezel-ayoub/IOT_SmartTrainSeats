/* ===================================================================
   Sensor_Check.ino  -  measure first, calibrate second.

   This sketch does NOTHING except read the two ultrasonic sensors and
   print what they see. No WiFi, no LEDs, no decisions. If something is
   wrong with the hardware, this is where you will see it.

   HOW TO USE
     1. Upload this sketch.
     2. Tools > Serial Monitor, and set the speed to 115200.
     3. With BOTH seats EMPTY, let it settle for 10 seconds and write
        down the "avg" number for each seat.   -> SEAT_EMPTY_CM
     4. Sit the doll on seat A, wait 10 seconds, write down "avg".
        Do the same for seat B.                -> SEAT_TAKEN_CM
     5. Put those two numbers at the top of seat_guide.ino.

   WHAT YOU SHOULD SEE
     Empty seat  : about 13 cm  (beam 17 cm above the floor, 4 cm cube)
     Doll sitting: about  6 cm
     The avg should barely move. If it jumps around by more than about
     1 cm, something in the beam is moving or the sensor is loose.

   IF IT SAYS "NO ECHO"
     The sensor sent a pulse and nothing came back within 30 ms.
     Check, in this order:
       - the 4 wires of that sensor are fully seated
       - VCC of the sensor is on 5V (VIN), not 3.3V
       - TRIG and ECHO are not swapped
       - nothing is touching the front of the two metal cylinders
   =================================================================== */

// --- Pins: these match your wiring. Do not change them. ---
const int TRIG_PINS[2] = {27, 13};   // seat A, seat B
const int ECHO_PINS[2] = {26, 35};

const int   WINDOW   = 20;    // how many readings the average covers
const int   INTERVAL = 400;   // ms between printed lines

float  samples[2][WINDOW];
int    count[2]   = {0, 0};
int    next_i[2]  = {0, 0};
unsigned long lastPrint = 0;

// Sends one pulse and returns the distance in cm, or 0 if nothing came back.
float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);   // give up after 30 ms
  if (duration == 0) return 0;                     // no echo
  return duration * 0.0343 / 2.0;                  // sound there and back
}

void addSample(int i, float d) {
  samples[i][next_i[i]] = d;
  next_i[i] = (next_i[i] + 1) % WINDOW;
  if (count[i] < WINDOW) count[i]++;
}

float average(int i) {
  if (count[i] == 0) return 0;
  float sum = 0;
  for (int k = 0; k < count[i]; k++) sum += samples[i][k];
  return sum / count[i];
}

float lowest(int i) {
  float lo = samples[i][0];
  for (int k = 1; k < count[i]; k++) if (samples[i][k] < lo) lo = samples[i][k];
  return lo;
}

float highest(int i) {
  float hi = samples[i][0];
  for (int k = 1; k < count[i]; k++) if (samples[i][k] > hi) hi = samples[i][k];
  return hi;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  for (int i = 0; i < 2; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }
  Serial.println();
  Serial.println("=== Sensor check ===");
  Serial.println("Empty seat should read about 13 cm.");
  Serial.println("Doll sitting should read about 6 cm.");
  Serial.println("Let the avg settle for 10 seconds before writing it down.");
  Serial.println();
}

void loop() {
  // Read the two sensors one after the other, never at the same time,
  // so one sensor cannot hear the other one's echo.
  float a = readDistance(TRIG_PINS[0], ECHO_PINS[0]);
  delay(60);
  float b = readDistance(TRIG_PINS[1], ECHO_PINS[1]);
  delay(60);

  if (a > 0) addSample(0, a);
  if (b > 0) addSample(1, b);

  if (millis() - lastPrint > INTERVAL) {
    lastPrint = millis();

    if (a > 0) Serial.printf("Seat A %6.1f cm  (avg %5.1f  min %5.1f  max %5.1f)",
                             a, average(0), lowest(0), highest(0));
    else       Serial.printf("Seat A    NO ECHO  - check the wiring of sensor A ");

    Serial.print("   |   ");

    if (b > 0) Serial.printf("Seat B %6.1f cm  (avg %5.1f  min %5.1f  max %5.1f)\n",
                             b, average(1), lowest(1), highest(1));
    else       Serial.printf("Seat B    NO ECHO  - check the wiring of sensor B\n");
  }
}
