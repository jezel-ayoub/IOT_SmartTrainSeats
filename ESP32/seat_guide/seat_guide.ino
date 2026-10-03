// ---------------------------------------------------------------
// Smart Train Seat Guide - main program
//
// Detects whether seats in a train car are taken, shows the result
// on a platform light strip and on a live web page, and recommends
// which car a waiting passenger should walk to.
//
// Car 1 is real (two ultrasonic sensors). Cars 2 and 3 are simulated
// from the web page, so a whole train can be demonstrated with one
// physical car.
//
// Board:   ESP32 DevKit V1 (DOIT)
// Sensors: 2 x HC-SR04 ultrasonic distance sensors
// Lights:  WS2812 (NeoPixel) strip, 3 LEDs
//
// Wiring is documented in Documentation/HARDWARE_WIRING.md
// ---------------------------------------------------------------

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// --- Fill these in with your own network before uploading. ---
// Do not commit real credentials to a public repository.
const char* WIFI_NAME     = "YOUR_HOTSPOT_NAME";
const char* WIFI_PASSWORD = "YOUR_HOTSPOT_PASSWORD";

// --- Pins ---
const int TRIG_PINS[2] = {27, 13};   // seat A, seat B
const int ECHO_PINS[2] = {26, 12};
const int LED_PIN = 14;              // data wire of the light strip

// --- Tuning ---
// Two different thresholds (hysteresis) stop a seat flickering between
// states when a reading sits exactly on the boundary.
const float TAKEN_BELOW = 35;            // cm: closer than this may become TAKEN
const float FREE_ABOVE  = 45;            // cm: farther than this may become FREE
const unsigned long STAY_TIME  = 2000;   // ms a change must persist to count
const unsigned long ERROR_TIME = 3000;   // ms without an echo = sensor fault
const float SMOOTHING = 0.7;             // exponential moving average weight

enum SeatState { FREE, TAKEN, SENSOR_ERROR };
SeatState seatState[2] = {FREE, FREE};
float smoothDist[2]    = {0, 0};
unsigned long changeStart[2] = {0, 0};
unsigned long lastEcho[2]    = {0, 0};

// Car 0 is the real car. Cars 1 and 2 are simulated.
int totalSeats[3] = {2, 10, 10};
int simFree[3]    = {0, 6, 2};

unsigned long lastWifiTry = 0;

Adafruit_NeoPixel strip(3, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// Sends one ultrasonic ping and returns the distance in cm, or 0 on timeout.
float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.0343 / 2.0;
}

void updateSeat(int i) {
  unsigned long now = millis();
  float d = readDistance(TRIG_PINS[i], ECHO_PINS[i]);

  if (d <= 0) {                                   // no echo returned
    if (now - lastEcho[i] > ERROR_TIME) seatState[i] = SENSOR_ERROR;
    return;
  }
  lastEcho[i] = now;

  if (smoothDist[i] == 0 || seatState[i] == SENSOR_ERROR) smoothDist[i] = d;
  else smoothDist[i] = SMOOTHING * smoothDist[i] + (1 - SMOOTHING) * d;

  if (seatState[i] == SENSOR_ERROR) {              // sensor recovered
    seatState[i] = (smoothDist[i] < TAKEN_BELOW) ? TAKEN : FREE;
    changeStart[i] = 0;
    return;
  }

  bool wantsChange = (seatState[i] == FREE  && smoothDist[i] < TAKEN_BELOW) ||
                     (seatState[i] == TAKEN && smoothDist[i] > FREE_ABOVE);
  if (!wantsChange) { changeStart[i] = 0; return; }

  if (changeStart[i] == 0) changeStart[i] = now;
  if (now - changeStart[i] >= STAY_TIME) {         // held long enough to be real
    seatState[i] = (seatState[i] == FREE) ? TAKEN : FREE;
    changeStart[i] = 0;
  }
}

bool realCarHasProblem() {
  return seatState[0] == SENSOR_ERROR || seatState[1] == SENSOR_ERROR;
}

int freeSeats(int car) {
  if (car > 0) return simFree[car];
  int n = 0;
  for (int i = 0; i < 2; i++) if (seatState[i] == FREE) n++;
  return n;
}

int bestCar() {
  int best = 0;
  for (int c = 1; c < 3; c++) if (freeSeats(c) > freeSeats(best)) best = c;
  return best;
}

void updateLights() {
  for (int c = 0; c < 3; c++) {
    uint32_t color;
    if (c == 0 && realCarHasProblem())            color = strip.Color(0, 0, 255);    // blue: fault
    else if (freeSeats(c) == 0)                   color = strip.Color(255, 0, 0);    // red: full
    else if (freeSeats(c) * 2 >= totalSeats[c])   color = strip.Color(0, 255, 0);    // green: plenty
    else                                          color = strip.Color(255, 110, 0);  // yellow: filling up
    strip.setPixelColor(c, color);
  }
  strip.show();
}

String carCard(int c) {
  String color, label;
  if (c == 0 && realCarHasProblem())            { color = "#7f8c8d"; label = "SENSOR PROBLEM"; }
  else if (freeSeats(c) == 0)                   { color = "#e74c3c"; label = "FULL"; }
  else if (freeSeats(c) * 2 >= totalSeats[c])   { color = "#2ecc71"; label = "PLENTY OF ROOM"; }
  else                                          { color = "#e67e22"; label = "FILLING UP"; }

  String s = "<div style='background:" + color + ";color:white;padding:18px;margin:12px 0;border-radius:16px'>";
  s += "<div style='font-size:26px;font-weight:bold'>Car " + String(c + 1);
  s += (c == 0) ? " (real)" : " (simulated)";
  s += "</div>";
  s += "<div style='font-size:34px;margin:6px 0'>" + String(freeSeats(c)) + " / " +
       String(totalSeats[c]) + " free</div>";
  s += "<div style='font-size:18px'>" + label + "</div>";
  if (c > 0) {
    s += "<div style='margin-top:10px'>";
    s += "<a href='/set?car=" + String(c) + "&d=-1' style='background:white;color:#333;padding:10px 20px;"
         "border-radius:10px;text-decoration:none;font-size:22px'>&minus;</a>&nbsp;&nbsp;";
    s += "<a href='/set?car=" + String(c) + "&d=1' style='background:white;color:#333;padding:10px 20px;"
         "border-radius:10px;text-decoration:none;font-size:22px'>+</a>";
    s += "</div>";
  }
  s += "</div>";
  return s;
}

void showPage() {
  String page = "<html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width'>"
                "<meta http-equiv='refresh' content='3'></head>"
                "<body style='font-family:sans-serif;max-width:480px;margin:auto;padding:12px'>"
                "<h1>Platform display</h1>";
  page += "<h2 style='color:#2c3e50'>Best car: Car " + String(bestCar() + 1) + "</h2>";
  for (int c = 0; c < 3; c++) page += carCard(c);
  page += "<p style='color:gray;font-size:13px'>Seat A: " + String(smoothDist[0]) +
          " cm &middot; Seat B: " + String(smoothDist[1]) + " cm</p>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

// Technician control for the simulated cars.
void handleSet() {
  int car = server.arg("car").toInt();
  int d   = server.arg("d").toInt();
  if (car >= 1 && car <= 2) {
    simFree[car] += d;
    if (simFree[car] < 0) simFree[car] = 0;
    if (simFree[car] > totalSeats[car]) simFree[car] = totalSeats[car];
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Reconnects in the background. Sensing and lights keep working meanwhile.
void keepWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWifiTry > 5000) {
    lastWifiTry = millis();
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 2; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }
  strip.begin();
  strip.setBrightness(40);
  strip.show();

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi...");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! Open: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No WiFi yet. Lights and sensors still work; will keep retrying.");
  }

  server.on("/", showPage);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  updateSeat(0);
  delay(60);                 // avoids one sensor hearing the other's echo
  updateSeat(1);
  delay(60);
  updateLights();
  keepWifi();
  server.handleClient();
}
