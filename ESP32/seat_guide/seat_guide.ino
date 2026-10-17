// ---------------------------------------------------------------
// Smart Train Seat Guide - main program (version 2)
//
// Car 1 is real: two HC-SR04 sensors, one per seat.
// Cars 2 and 3 are simulated and are controlled by tapping a seat
// on the platform display.
//
// Serves:
//   /        the platform display page
//   /data    the seat states, as JSON
//   /toggle  flips one simulated seat
//
// Colour rule for the platform lights and the page:
//   more than half the seats free -> green
//   some free but half or fewer   -> yellow
//   none free                     -> red
//   a sensor has failed           -> blue
//
// Board:   ESP32 DevKit V1 (DOIT)
// Sensors: 2 x HC-SR04      Lights: WS2812 strip, 3 LEDs
// Wiring is documented in Documentation/HARDWARE_WIRING.md
// ---------------------------------------------------------------

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// --- Network. Replace with placeholders before uploading this file to GitHub. ---
const char* WIFI_NAME     = "YOUR_HOTSPOT_NAME";
const char* WIFI_PASSWORD = "YOUR_HOTSPOT_PASSWORD";

// --- Pins ---
const int TRIG_PINS[2] = {27, 13};   // seat A, seat B
const int ECHO_PINS[2] = {26, 35};   // D35 is input-only: ideal for an echo line
const int LED_PIN = 14;

// ===================================================================
//  THRESHOLDS - ONE PAIR PER SEAT.
//
//  Seat A is index 0, seat B is index 1.
//  TAKEN_BELOW : a reading SMALLER than this means someone is sitting.
//  FREE_ABOVE  : a reading BIGGER  than this means the seat is empty.
//
//  THE RULE THAT MATTERS:
//    the seat's EMPTY reading must be ABOVE its FREE_ABOVE number.
//    If the empty reading lands between the two numbers, the seat can
//    go to TAKEN but can never come back - it gets stuck.
//
//  Read each seat's empty number off the Serial Monitor, then set
//  FREE_ABOVE about 2 cm below it, and TAKEN_BELOW 2 cm below that.
// ===================================================================
//                              seat A   seat B
const float TAKEN_BELOW[2] = {    8.5,     8.5  };   // cm
const float FREE_ABOVE[2]  = {   10.5,    10.5  };   // cm

// --- Tuning ---
const int   PINGS        = 5;      // readings per measurement; the middle one is used
const float MIN_VALID_CM = 2.5;    // anything closer is a reflection, not a seat
const float MAX_VALID_CM = 25.0;   // anything farther is past the floor
const unsigned long STAY_TIME  = 2000;   // ms a change must persist to count
const unsigned long ERROR_TIME = 3000;   // ms without an echo = sensor fault
const float SMOOTHING = 0.5;             // the median is already clean, so smooth less

enum SeatState { FREE_SEAT, TAKEN_SEAT, FAULT_SEAT };
SeatState seatState[2] = {FREE_SEAT, FREE_SEAT};
float smoothDist[2]    = {0, 0};
float lastRaw[2]       = {0, 0};
unsigned long changeStart[2] = {0, 0};
unsigned long lastEcho[2]    = {0, 0};

// --- The train ---
const int CAR_COUNT = 3;
const int MAX_SEATS = 10;
const int CAR_SEATS[CAR_COUNT] = {2, 10, 10};   // car 0 is the real one
bool simTaken[CAR_COUNT][MAX_SEATS];

unsigned long lastWifiTry = 0;
unsigned long lastPrint = 0;

Adafruit_NeoPixel strip(CAR_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

const char PAGE_HTML[] PROGMEM = R"HTML(
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Platform Seat Display</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Rubik:wght@400;500;600;700&display=swap">
<link rel="stylesheet" href="/s.css">

<div class="wrap">

  <div class="topbar">
    <div class="station">Central Station &middot; Platform 2<small>SMART SEAT GUIDE</small></div>
    <div class="spacer"></div>
    <div class="eta">Train arriving in <b id="eta">02:00</b></div>
    <div class="clock" id="clock">00:00</div>
    <span class="link" id="link"><i></i><span id="linktext">Connected to controller</span></span>
  </div>

  <div class="rec">
    <div>
      <div class="big">Board <em id="recName">Car 2</em></div>
      <div class="sub" id="recSub">The car with the most free seats on this train</div>
    </div>
  </div>

  <div class="trainwrap">
    <div class="train" id="train"></div>
    <div class="platform"></div>
  </div>

  <div class="grid2">
    <div class="panel">
      <h2>Legend</h2>
      <div class="legend">
        <span><i class="sw" style="background:var(--free)"></i> Seat free</span>
        <span><i class="sw" style="background:var(--full)"></i> Seat taken</span>
        <span><i class="sw" style="background:var(--fault)"></i> Sensor fault</span>
        <span><i class="sw" style="background:var(--amber)"></i> Recommended car</span>
      </div>
    </div>
    <div class="panel">
      <h2>Technician view</h2>
      <p class="hint">Car 1 is the <b>pilot car</b>: both of its seats have real distance sensors and update by themselves. In cars 2 and 3 you can <b>tap a seat</b> to seat or remove a passenger, so a whole train can be demonstrated with one instrumented car.</p>
    </div>
  </div>

</div>

<script src="/a.js"></script>

)HTML";

const char PAGE_CSS[] PROGMEM = R"CSS(

  /* Platform information display: dark board, train drawn top-down, cars left to right. */
  :root {
    color-scheme: dark;
    --bg: #0b1117;
    --board: #121c24;
    --board-2: #18242e;
    --line: #243441;
    --fg: #eaf2f6;
    --dim: #8fa6b4;
    --amber: #ffc24d;
    --free: #27c279;
    --few: #f0a62b;
    --full: #e2584a;
    --fault: #7d8f9b;
    --metal-1: #cfd9df;
    --metal-2: #9fb0ba;
    --font: "Rubik", "Heebo", "Arial Hebrew", system-ui, sans-serif;
  }

  html, body { height: 100%; }
  body {
    margin: 0; background:
      radial-gradient(1200px 600px at 50% -10%, #16232e 0%, transparent 60%),
      var(--bg);
    color: var(--fg); font-family: var(--font);
    font-size: 16px; line-height: 1.5;
    padding-inline: 16px; padding-block: 18px 34px;
  }
  .wrap { max-width: 1180px; margin: 0 auto; display: flex; flex-direction: column; gap: 16px; }

  /* ---------- top bar ---------- */
  .topbar {
    display: flex; flex-wrap: wrap; align-items: center; gap: 12px 18px;
    background: linear-gradient(180deg, var(--board-2), var(--board));
    border: 1px solid var(--line); border-radius: 16px; padding: 14px 18px;
  }
  .station { font-size: clamp(19px, 3.2vw, 26px); font-weight: 700; letter-spacing: -0.01em; }
  .station small { display: block; font-size: 12px; font-weight: 500; color: var(--dim); letter-spacing: .08em; }
  .spacer { flex: 1 1 auto; }
  .clock { font-variant-numeric: tabular-nums; font-size: clamp(20px, 3.4vw, 28px); font-weight: 600; color: var(--amber); }
  .eta { font-size: 14px; color: var(--dim); }
  .eta b { color: var(--fg); font-variant-numeric: tabular-nums; }
  .link {
    display: inline-flex; align-items: center; gap: 7px; font-size: 12.5px; color: var(--dim);
    border: 1px solid var(--line); border-radius: 999px; padding: 5px 11px; background: #0e171e;
  }
  .link i { width: 8px; height: 8px; border-radius: 50%; background: var(--free); display: inline-block; }
  .link.off i { background: var(--amber); }

  /* ---------- recommendation ---------- */
  .rec {
    background: linear-gradient(180deg, #16303a, #112630);
    border: 1px solid #1f4a52; border-radius: 16px;
    padding: 16px 20px; display: flex; align-items: center; gap: 16px; flex-wrap: wrap;
  }
  .rec .big { font-size: clamp(20px, 4vw, 30px); font-weight: 700; }
  .rec .big em { font-style: normal; color: var(--amber); }
  .rec .sub { color: var(--dim); font-size: 14px; }

  /* ---------- train ---------- */
  .trainwrap { overflow-x: auto; padding-bottom: 4px; direction: ltr; }
  .train { direction: ltr; display: flex; align-items: stretch; gap: 10px; min-width: 760px; }

  .loco {
    flex: 0 0 76px; align-self: stretch; border-radius: 34px 10px 10px 34px;
    background: linear-gradient(180deg, var(--metal-1), var(--metal-2));
    position: relative; margin-top: 29px; margin-bottom: 26px;
    box-shadow: inset 0 -8px 16px rgba(0,0,0,.25);
  }
  .loco::before {
    content: ""; position: absolute; inset: 16px auto 16px 14px; width: 26px; border-radius: 16px 6px 6px 16px;
    background: #16242d; opacity: .85;
  }
  .loco::after {
    content: ""; position: absolute; left: 6px; top: 50%; transform: translateY(-50%);
    width: 10px; height: 10px; border-radius: 50%; background: var(--amber);
    box-shadow: 0 0 14px var(--amber);
  }

  .car { flex: 1 1 0; min-width: 190px; display: flex; flex-direction: column; gap: 6px; }

  .carhead { display: flex; align-items: baseline; gap: 8px; padding-inline: 4px; }
  .carname { font-weight: 600; font-size: 15px; }
  .carcount { font-size: 13px; color: var(--dim); font-variant-numeric: tabular-nums; }
  .pill {
    margin-inline-start: auto; font-size: 11.5px; font-weight: 600; letter-spacing: .02em;
    padding: 3px 9px; border-radius: 999px; background: #0e171e; border: 1px solid var(--line);
  }

  .body {
    position: relative; border-radius: 14px; padding: 10px 12px 12px;
    background: linear-gradient(180deg, #e8eef1 0%, #cfdae0 48%, #b9c8d0 100%);
    box-shadow: inset 0 -10px 18px rgba(0,0,0,.18);
    border: 1px solid #93a6b0;
  }
  .stripe { height: 7px; border-radius: 999px; margin-bottom: 9px; background: var(--fault); transition: background .4s; }
  .doors { position: absolute; inset-block: 14px; width: 7px; border-radius: 4px; background: #8ea0aa; }
  .doors.l { left: -1px; } .doors.r { right: -1px; }

  .deck { display: flex; flex-direction: column; gap: 7px; }
  .row { display: flex; gap: 6px; justify-content: center; min-height: 42px; }
  .aisle { height: 10px; border-radius: 999px; background: repeating-linear-gradient(90deg, #a9bac3 0 10px, transparent 10px 20px); opacity: .7; }

  .seat {
    position: relative; flex: 1 1 0; max-width: 46px; aspect-ratio: 1 / 1.08;
    border-radius: 7px 7px 10px 10px; background: var(--free);
    border: none; padding: 0; cursor: default; transition: background .35s, transform .15s;
    box-shadow: inset 0 -3px 0 rgba(0,0,0,.18);
  }
  .seat::before {
    content: ""; position: absolute; left: 14%; right: 14%; top: 10%; height: 26%;
    border-radius: 6px 6px 3px 3px; background: rgba(255,255,255,.34);
  }
  .seat.taken { background: var(--full); }
  .seat.fault { background: var(--fault); }
  .seat.taken::after {
    content: ""; position: absolute; left: 50%; top: 46%; transform: translateX(-50%);
    width: 42%; aspect-ratio: 1; border-radius: 50%; background: rgba(255,255,255,.9);
    box-shadow: 0 10px 0 -2px rgba(255,255,255,.85);
  }
  .seat.sim { cursor: pointer; }
  .seat.sim:hover { transform: translateY(-2px); }
  .seat.sim:focus-visible { outline: 3px solid var(--amber); outline-offset: 3px; }

  .seat.live { outline: 2px solid #0b1117; outline-offset: 2px; }
  .seat.live .tag {
    position: absolute; left: 0; right: 0; bottom: 6%;
    font-size: 11px; font-weight: 700; color: #0b1117; text-align: center;
  }
  .seat.live .dot {
    position: absolute; inset-inline-end: 5px; top: 5px; width: 7px; height: 7px; border-radius: 50%;
    background: #0b1117; animation: blip 1.8s ease-in-out infinite;
  }
  @keyframes blip { 0%,100% { opacity: .25 } 50% { opacity: 1 } }

  .car.best .body { box-shadow: 0 0 0 2px var(--amber), 0 0 26px rgba(255,194,77,.3), inset 0 -10px 18px rgba(0,0,0,.18); }
  .arrow {
    height: 26px; display: grid; place-items: center; color: var(--amber);
    font-size: 20px; opacity: 0; transition: opacity .3s;
  }
  .car.best .arrow { opacity: 1; animation: nudge 1.6s ease-in-out infinite; }
  @keyframes nudge { 0%,100% { transform: translateY(0) } 50% { transform: translateY(5px) } }

  .platform {
    margin-top: 2px; height: 26px; border-radius: 10px; min-width: 760px;
    background: repeating-linear-gradient(90deg, #1b2a34 0 26px, #203140 26px 52px);
    border-top: 5px solid var(--amber);
  }

  /* ---------- legend + panel ---------- */
  .grid2 { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 14px; }
  .panel { background: var(--board); border: 1px solid var(--line); border-radius: 16px; padding: 15px 17px; }
  .panel h2 { margin: 0 0 10px; font-size: 15px; font-weight: 600; letter-spacing: .02em; }
  .legend { display: flex; flex-wrap: wrap; gap: 9px 16px; font-size: 13.5px; color: var(--dim); }
  .legend span { display: inline-flex; align-items: center; gap: 7px; }
  .sw { width: 13px; height: 13px; border-radius: 4px; display: inline-block; }
  .hint { margin: 0; font-size: 13.5px; color: var(--dim); }
  .hint b { color: var(--fg); font-weight: 600; }

)CSS";

const char PAGE_JS[] PROGMEM = R"JS(

  // Seat codes: 0 = free, 1 = taken, 2 = sensor fault
  var CARS = [
    { name: "Car 1", live: true,  seats: [0, 0] },
    { name: "Car 2", live: false, seats: [0,0,1,0,0, 1,0,0,0,0] },
    { name: "Car 3", live: false, seats: [1,1,1,0,1, 1,1,1,0,1] }
  ];
  var connected = false;          // true once the board answers /data
  var LIVE_LABELS = ["A", "B"];

  function freeOf(c) { var n = 0; for (var i = 0; i < c.seats.length; i++) if (c.seats[i] === 0) n++; return n; }
  function faulty(c) { for (var i = 0; i < c.seats.length; i++) if (c.seats[i] === 2) return true; return false; }

  // More than half free = green. Exactly half or fewer = yellow. None = red.
  function statusOf(c) {
    if (faulty(c)) return { key: "fault", color: "var(--fault)", text: "Sensor fault" };
    var f = freeOf(c), t = c.seats.length;
    if (f === 0) return { key: "full", color: "var(--full)", text: "Full" };
    if (f * 2 > t) return { key: "free", color: "var(--free)", text: "Plenty of room" };
    return { key: "few", color: "var(--few)", text: "Filling up" };
  }

  function bestCar() {
    var best = 0;
    for (var i = 1; i < CARS.length; i++) if (freeOf(CARS[i]) > freeOf(CARS[best])) best = i;
    return freeOf(CARS[best]) === 0 ? -1 : best;
  }

  var trainEl = document.getElementById("train");

  function build() {
    trainEl.innerHTML = "";
    var loco = document.createElement("div");
    loco.className = "loco";
    trainEl.appendChild(loco);

    CARS.forEach(function (car, ci) {
      var el = document.createElement("div");
      el.className = "car";
      el.dataset.car = ci;

      var head = document.createElement("div");
      head.className = "carhead";
      head.innerHTML = '<span class="carname">' + car.name + '</span>' +
                       '<span class="carcount" data-count></span>' +
                       '<span class="pill" data-pill></span>';
      el.appendChild(head);

      var body = document.createElement("div");
      body.className = "body";
      body.innerHTML = '<div class="doors l"></div><div class="doors r"></div><div class="stripe" data-stripe></div>';

      var deck = document.createElement("div");
      deck.className = "deck";
      var half = Math.ceil(car.seats.length / 2);
      var rowA = document.createElement("div"); rowA.className = "row";
      var aisle = document.createElement("div"); aisle.className = "aisle";
      var rowB = document.createElement("div"); rowB.className = "row";

      car.seats.forEach(function (_, si) {
        var s = document.createElement("button");
        s.type = "button";
        s.dataset.seat = si;
        if (car.live) {
          s.className = "seat live";
          s.innerHTML = '<span class="dot"></span><span class="tag">' + (LIVE_LABELS[si] || "") + '</span>';
          s.setAttribute("aria-label", "Seat " + (LIVE_LABELS[si] || "") + ", live sensor");
          s.disabled = true;
        } else {
          s.className = "seat sim";
          s.setAttribute("aria-label", car.name + ", seat " + (si + 1) + ", tap to change");
          s.addEventListener("click", function () { toggle(ci, si); });
        }
        (si < half ? rowA : rowB).appendChild(s);
      });

      deck.appendChild(rowA); deck.appendChild(aisle); deck.appendChild(rowB);
      body.appendChild(deck);
      el.appendChild(body);

      var arrow = document.createElement("div");
      arrow.className = "arrow"; arrow.textContent = "\u25B2";
      el.appendChild(arrow);

      trainEl.appendChild(el);
    });
  }

  function paint() {
    var best = bestCar();
    CARS.forEach(function (car, ci) {
      var el = trainEl.querySelector('.car[data-car="' + ci + '"]');
      var st = statusOf(car);
      el.classList.toggle("best", ci === best);
      el.querySelector("[data-stripe]").style.background = st.color;
      el.querySelector("[data-pill]").textContent = st.text;
      el.querySelector("[data-pill]").style.color = st.color;
      el.querySelector("[data-count]").textContent = freeOf(car) + " / " + car.seats.length + " free";
      car.seats.forEach(function (v, si) {
        var s = el.querySelector('[data-seat="' + si + '"]');
        s.classList.toggle("taken", v === 1);
        s.classList.toggle("fault", v === 2);
      });
    });

    var rn = document.getElementById("recName"), rs = document.getElementById("recSub");
    if (best < 0) {
      rn.textContent = "No free seats";
      rs.textContent = "Every car on this train is full";
    } else {
      rn.textContent = CARS[best].name;
      rs.textContent = freeOf(CARS[best]) + " seats free \u2014 the most on this train";
    }
  }

  function toggle(ci, si) {
    CARS[ci].seats[si] = CARS[ci].seats[si] === 1 ? 0 : 1;
    paint();
    if (connected) {
      fetch("/toggle?car=" + ci + "&seat=" + si).catch(function () {});
    }
  }

  // ---- live data from the board; falls back to demo mode when served elsewhere ----
  function pull() {
    fetch("/data", { cache: "no-store" })
      .then(function (r) { return r.json(); })
      .then(function (d) {
        if (!d || !d.cars) return;
        d.cars.forEach(function (c, i) { if (CARS[i]) CARS[i].seats = c.seats; });
        if (!connected) { connected = true; setLink(true); }
        paint();
      })
      .catch(function () { if (connected || !linkSet) { connected = false; setLink(false); } });
  }

  var linkSet = false;
  function setLink(on) {
    linkSet = true;
    var el = document.getElementById("link");
    el.classList.toggle("off", !on);
    document.getElementById("linktext").textContent = on ? "Connected to controller" : "Demo mode";
  }

  // ---- station chrome ----
  function tick() {
    var now = new Date();
    document.getElementById("clock").textContent =
      String(now.getHours()).padStart(2, "0") + ":" + String(now.getMinutes()).padStart(2, "0");
  }
  var eta = 120;
  function etaTick() {
    eta = eta <= 0 ? 180 : eta - 1;
    document.getElementById("eta").textContent =
      String(Math.floor(eta / 60)).padStart(2, "0") + ":" + String(eta % 60).padStart(2, "0");
  }

  build();
  paint();
  tick(); etaTick();
  setInterval(tick, 10000);
  setInterval(etaTick, 1000);
  pull();
  setInterval(pull, 1000);

)JS";

// One ping. Returns cm, or 0 if nothing came back.
float onePing(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 0;
  return duration * 0.0343 / 2.0;
}

// Takes PINGS readings and returns the MIDDLE one, not the average.
// A stray echo - off the backrest, off a wall, off the other sensor -
// shows up as one odd value out of five, and the middle value ignores
// it completely. An average would be dragged by it; a median is not.
float readDistance(int trigPin, int echoPin) {
  float v[PINGS];
  int n = 0;
  for (int k = 0; k < PINGS; k++) {
    float d = onePing(trigPin, echoPin);
    if (d >= MIN_VALID_CM && d <= MAX_VALID_CM) v[n++] = d;
    delay(6);
  }
  if (n == 0) return 0;                       // nothing sensible came back
  for (int a = 1; a < n; a++) {               // sort the few values we have
    float key = v[a];
    int b = a - 1;
    while (b >= 0 && v[b] > key) { v[b + 1] = v[b]; b--; }
    v[b + 1] = key;
  }
  return v[n / 2];                            // the middle one
}

void updateSeat(int i) {
  unsigned long now = millis();
  float d = readDistance(TRIG_PINS[i], ECHO_PINS[i]);

  if (d <= 0) {                                    // no echo returned
    if (now - lastEcho[i] > ERROR_TIME) seatState[i] = FAULT_SEAT;
    return;
  }
  lastEcho[i] = now;
  lastRaw[i]  = d;

  if (smoothDist[i] == 0 || seatState[i] == FAULT_SEAT) smoothDist[i] = d;
  else smoothDist[i] = SMOOTHING * smoothDist[i] + (1 - SMOOTHING) * d;

  if (seatState[i] == FAULT_SEAT) {                // sensor recovered
    seatState[i] = (smoothDist[i] < TAKEN_BELOW[i]) ? TAKEN_SEAT : FREE_SEAT;
    changeStart[i] = 0;
    return;
  }

  bool wantsChange = (seatState[i] == FREE_SEAT  && smoothDist[i] < TAKEN_BELOW[i]) ||
                     (seatState[i] == TAKEN_SEAT && smoothDist[i] > FREE_ABOVE[i]);
  if (!wantsChange) { changeStart[i] = 0; return; }

  if (changeStart[i] == 0) changeStart[i] = now;
  if (now - changeStart[i] >= STAY_TIME) {         // held long enough to be real
    seatState[i] = (seatState[i] == FREE_SEAT) ? TAKEN_SEAT : FREE_SEAT;
    changeStart[i] = 0;
  }
}

// 0 = free, 1 = taken, 2 = sensor fault
int seatCode(int car, int seat) {
  if (car == 0) {
    if (seatState[seat] == FAULT_SEAT) return 2;
    return seatState[seat] == TAKEN_SEAT ? 1 : 0;
  }
  return simTaken[car][seat] ? 1 : 0;
}

bool carHasFault(int car) {
  for (int s = 0; s < CAR_SEATS[car]; s++) if (seatCode(car, s) == 2) return true;
  return false;
}

int freeSeats(int car) {
  int n = 0;
  for (int s = 0; s < CAR_SEATS[car]; s++) if (seatCode(car, s) == 0) n++;
  return n;
}

int bestCar() {
  int best = 0;
  for (int c = 1; c < CAR_COUNT; c++) if (freeSeats(c) > freeSeats(best)) best = c;
  return freeSeats(best) == 0 ? -1 : best;
}

void updateLights() {
  for (int c = 0; c < CAR_COUNT; c++) {
    uint32_t color;
    int f = freeSeats(c), t = CAR_SEATS[c];
    if (carHasFault(c))        color = strip.Color(0, 0, 255);      // blue:   sensor fault
    else if (f == 0)           color = strip.Color(255, 0, 0);      // red:    full
    else if (f * 2 > t)        color = strip.Color(0, 255, 0);      // green:  more than half free
    else                       color = strip.Color(255, 110, 0);    // yellow: half or fewer free
    strip.setPixelColor(c, color);
  }
  strip.show();
}

void handlePage() { server.send_P(200, "text/html; charset=utf-8", PAGE_HTML); }
void handleCss()  { server.send_P(200, "text/css; charset=utf-8", PAGE_CSS); }
void handleJs()   { server.send_P(200, "application/javascript; charset=utf-8", PAGE_JS); }

void handleData() {
  String j = "{\"best\":" + String(bestCar()) + ",\"cars\":[";
  for (int c = 0; c < CAR_COUNT; c++) {
    if (c) j += ",";
    j += "{\"seats\":[";
    for (int s = 0; s < CAR_SEATS[c]; s++) {
      if (s) j += ",";
      j += String(seatCode(c, s));
    }
    j += "]}";
  }
  j += "]}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", j);
}

void handleToggle() {
  int car  = server.arg("car").toInt();
  int seat = server.arg("seat").toInt();
  if (car >= 1 && car < CAR_COUNT && seat >= 0 && seat < CAR_SEATS[car]) {
    simTaken[car][seat] = !simTaken[car][seat];
  }
  server.send(200, "text/plain", "ok");
}

// Reconnects in the background. Sensing and lights keep working meanwhile.
void keepWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWifiTry > 10000) {
    lastWifiTry = millis();
    Serial.println("WiFi not connected, retrying...");
    WiFi.disconnect();
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  }
}

// Turns a seat state into a word for the Serial Monitor.
const char* stateName(SeatState st) {
  if (st == TAKEN_SEAT) return "TAKEN";
  if (st == FAULT_SEAT) return "FAULT";
  return "FREE";
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== Smart Train Seat Guide ===");
  Serial.printf("Seat A:  TAKEN below %.1f cm,  FREE above %.1f cm\n", TAKEN_BELOW[0], FREE_ABOVE[0]);
  Serial.printf("Seat B:  TAKEN below %.1f cm,  FREE above %.1f cm\n", TAKEN_BELOW[1], FREE_ABOVE[1]);
  Serial.println("Each seat's EMPTY reading must be ABOVE its FREE number.");
  Serial.println("If it is not, that seat will stick on TAKEN.");
  Serial.println();

  for (int i = 0; i < 2; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }

  // A plausible starting state for the simulated cars
  for (int c = 0; c < CAR_COUNT; c++)
    for (int s = 0; s < MAX_SEATS; s++) simTaken[c][s] = false;
  simTaken[1][2] = true; simTaken[1][5] = true;
  for (int s = 0; s < 10; s++) simTaken[2][s] = true;
  simTaken[2][3] = false; simTaken[2][8] = false;

  strip.begin();
  strip.setBrightness(40);
  strip.show();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(200);
  WiFi.setSleep(false);          // keeps the connection steady on a phone hotspot
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_NAME);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("PASS - connected. Open this address in a browser: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No WiFi yet. Lights and sensors still work; will keep retrying.");
  }

  server.on("/", handlePage);
  server.on("/s.css", handleCss);
  server.on("/a.js", handleJs);
  server.on("/data", handleData);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  updateSeat(0);
  delay(100);                // lets every echo die before the other sensor fires
  updateSeat(1);
  delay(100);
  updateLights();
  keepWifi();
  server.handleClient();

  // live readout, so the Serial Monitor shows both distance and decision
  if (millis() - lastPrint > 400) {
    lastPrint = millis();
    Serial.printf("Seat A  raw %5.1f  avg %5.1f  %-5s   |   Seat B  raw %5.1f  avg %5.1f  %-5s\n",
                  lastRaw[0], smoothDist[0], stateName(seatState[0]),
                  lastRaw[1], smoothDist[1], stateName(seatState[1]));
  }
}
