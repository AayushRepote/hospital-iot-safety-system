/*
  ============================================================
      HOSPITAL IoT SAFETY & EMERGENCY MONITORING SYSTEM
      ESP32 + WiFi + Web Dashboard

      Sensors:
      1. DS18B20 Temperature
      2. Flame/Fire Sensor
      3. MQ-2 Smoke Sensor
      4. Vibration Sensor

      Output:
      5. OLED SSD1306
      6. Buzzer

      Website pages:
      /              Dashboard
      /monitoring    Live Monitoring
      /events        Emergency Events
      /analytics     Analytics
      /system        System Status
      /about         About Project

      API:
      /api/data
  ============================================================
*/


#include <WiFi.h>
#include <WebServer.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <OneWire.h>
#include <DallasTemperature.h>


// ============================================================
// WIFI
// ============================================================



// ============================================================
// PIN CONFIGURATION
// ============================================================

#define TEMP_PIN        4
#define FIRE_PIN        27

#define SMOKE_DO_PIN    26
#define SMOKE_AO_PIN    34

#define VIBRATION_PIN   25

#define BUZZER_PIN      23


// ============================================================
// OLED
// ============================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// ============================================================
// DS18B20
// ============================================================

OneWire oneWire(TEMP_PIN);

DallasTemperature tempSensor(
  &oneWire
);


// ============================================================
// WEB SERVER
// ============================================================

WebServer server(80);


// ============================================================
// SENSOR DATA
// ============================================================

float temperature = 0.0;

int smokeAnalog = 0;

bool fireDetected = false;

bool smokeDetected = false;

bool vibrationDetected = false;

bool alarmActive = false;


// ============================================================
// THRESHOLDS
// ============================================================

float HIGH_TEMPERATURE =
  40.0;

int SMOKE_THRESHOLD =
  1500;


// ============================================================
// SYSTEM INFORMATION
// ============================================================

unsigned long systemStartTime = 0;

unsigned long lastSensorRead = 0;

unsigned long lastOLEDUpdate = 0;


// ============================================================
// EVENT SYSTEM
// ============================================================

#define MAX_EVENTS 20


String eventMessages[MAX_EVENTS];

String eventTimes[MAX_EVENTS];

int eventCount = 0;


// ============================================================
// PREVIOUS STATES
// ============================================================

bool previousFire = false;

bool previousSmoke = false;

bool previousVibration = false;

bool previousAlarm = false;


// ============================================================
// ADD EVENT
// ============================================================

void addEvent(String message) {

  String currentTime =
    String(millis() / 1000);

  if (eventCount < MAX_EVENTS) {

    eventMessages[eventCount] =
      message;

    eventTimes[eventCount] =
      currentTime;

    eventCount++;

  }

  else {

    for (
      int i = 0;
      i < MAX_EVENTS - 1;
      i++
    ) {

      eventMessages[i] =
        eventMessages[i + 1];

      eventTimes[i] =
        eventTimes[i + 1];

    }

    eventMessages[MAX_EVENTS - 1] =
      message;

    eventTimes[MAX_EVENTS - 1] =
      currentTime;
  }
}


// ============================================================
// READ SENSORS
// ============================================================

void readSensors() {

  // --------------------------
  // FIRE
  // --------------------------

  int fireValue =
    digitalRead(FIRE_PIN);

  fireDetected =
    (fireValue == LOW);


  // --------------------------
  // SMOKE
  // --------------------------

  smokeAnalog =
    analogRead(SMOKE_AO_PIN);

  int smokeDigital =
    digitalRead(SMOKE_DO_PIN);

  smokeDetected =
    (
      smokeDigital == LOW
    )
    ||
    (
      smokeAnalog >=
      SMOKE_THRESHOLD
    );


  // --------------------------
  // VIBRATION
  // --------------------------

  int vibrationValue =
    digitalRead(
      VIBRATION_PIN
    );

  vibrationDetected =
    (
      vibrationValue == HIGH
    );


  // --------------------------
  // TEMPERATURE
  // --------------------------

  tempSensor.requestTemperatures();

  temperature =
    tempSensor.getTempCByIndex(0);


  // --------------------------
  // ALARM
  // --------------------------

  alarmActive =
      fireDetected
      ||
      smokeDetected
      ||
      vibrationDetected
      ||
      (
        temperature !=
        DEVICE_DISCONNECTED_C
        &&
        temperature >=
        HIGH_TEMPERATURE
      );


  // --------------------------
  // EVENTS
  // --------------------------

  if (
    fireDetected &&
    !previousFire
  ) {

    addEvent(
      "🔥 Fire detected"
    );

  }


  if (
    smokeDetected &&
    !previousSmoke
  ) {

    addEvent(
      "💨 Smoke detected"
    );

  }


  if (
    vibrationDetected &&
    !previousVibration
  ) {

    addEvent(
      "📳 Vibration detected"
    );

  }


  if (
    temperature >=
    HIGH_TEMPERATURE
    &&
    temperature !=
    DEVICE_DISCONNECTED_C
  ) {

    if (
      !previousAlarm
      ||
      !alarmActive
    ) {

      addEvent(
        "🌡️ High temperature"
      );

    }

  }


  if (
    alarmActive &&
    !previousAlarm
  ) {

    addEvent(
      "🚨 Emergency alarm activated"
    );

  }


  if (
    !alarmActive &&
    previousAlarm
  ) {

    addEvent(
      "✅ Emergency condition cleared"
    );

  }


  previousFire =
    fireDetected;

  previousSmoke =
    smokeDetected;

  previousVibration =
    vibrationDetected;

  previousAlarm =
    alarmActive;
}


// ============================================================
// BUZZER
// ============================================================

void updateBuzzer() {

  if (alarmActive) {

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

  }

  else {

    digitalWrite(
      BUZZER_PIN,
      LOW
    );
  }
}


// ============================================================
// OLED
// ============================================================

void updateOLED() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);


  display.setCursor(
    0,
    0
  );

  display.println(
    "HOSPITAL SAFETY"
  );


  display.drawLine(
    0,
    9,
    127,
    9,
    SSD1306_WHITE
  );


  // Temperature

  display.setCursor(
    0,
    13
  );

  display.print(
    "TEMP: "
  );


  if (
    temperature ==
    DEVICE_DISCONNECTED_C
  ) {

    display.println(
      "ERROR"
    );

  }

  else {

    display.print(
      temperature,
      1
    );

    display.println(
      " C"
    );
  }


  // Fire

  display.setCursor(
    0,
    25
  );

  display.print(
    "FIRE: "
  );


  if (fireDetected) {

    display.println(
      "DETECTED"
    );

  }

  else {

    display.println(
      "SAFE"
    );
  }


  // Smoke

  display.setCursor(
    0,
    37
  );

  display.print(
    "SMOKE: "
  );


  if (smokeDetected) {

    display.println(
      "DETECTED"
    );

  }

  else {

    display.println(
      "SAFE"
    );
  }


  // Vibration

  display.setCursor(
    0,
    49
  );

  display.print(
    "VIB: "
  );


  if (vibrationDetected) {

    display.println(
      "DETECTED"
    );

  }

  else {

    display.println(
      "NORMAL"
    );
  }


  display.display();
}


// ============================================================
// WEBSITE HTML
// ============================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>
Hospital IoT Safety System
</title>


<style>

/* ========================================================
   GLOBAL
======================================================== */

* {
  box-sizing:border-box;
}

body {

  margin:0;

  font-family:
    Arial,
    Helvetica,
    sans-serif;

  background:#f1f5f9;

  color:#0f172a;

}


/* ========================================================
   SIDEBAR
======================================================== */

.sidebar {

  position:fixed;

  left:0;

  top:0;

  bottom:0;

  width:240px;

  background:#0f172a;

  color:white;

  padding:20px;

  z-index:100;

}


.logo {

  text-align:center;

  padding:15px 5px 25px;

  border-bottom:
    1px solid #334155;

}


.logo-icon {

  font-size:35px;

}


.logo h2 {

  margin:
    8px 0 3px;

  font-size:18px;

}


.logo p {

  margin:0;

  color:#94a3b8;

  font-size:11px;

}


.menu-title {

  margin-top:25px;

  color:#64748b;

  font-size:10px;

  text-transform:uppercase;

}


.nav {

  display:block;

  margin-top:8px;

  padding:13px;

  border-radius:8px;

  color:#cbd5e1;

  text-decoration:none;

  cursor:pointer;

}


.nav:hover {

  background:#1e293b;

  color:white;

}


.nav.active {

  background:#2563eb;

  color:white;

}


/* ========================================================
   MAIN
======================================================== */

.main {

  margin-left:240px;

  min-height:100vh;

}


/* ========================================================
   HEADER
======================================================== */

.header {

  height:70px;

  background:white;

  border-bottom:
    1px solid #e2e8f0;

  display:flex;

  align-items:center;

  justify-content:space-between;

  padding:0 30px;

}


.header h1 {

  margin:0;

  font-size:21px;

}


.header p {

  margin:
    3px 0 0;

  color:#64748b;

  font-size:12px;

}


.connection {

  color:#16a34a;

  font-size:13px;

}


/* ========================================================
   CONTENT
======================================================== */

.content {

  padding:30px;

}


/* ========================================================
   PAGE
======================================================== */

.page {

  display:none;

}


.page.active {

  display:block;

}


/* ========================================================
   HERO
======================================================== */

.hero {

  background:
    linear-gradient(
      135deg,
      #16a34a,
      #15803d
    );

  color:white;

  border-radius:15px;

  padding:25px;

  margin-bottom:25px;

}


.hero.alarm {

  background:
    linear-gradient(
      135deg,
      #dc2626,
      #991b1b
    );

  animation:
    blink 1s infinite;

}


@keyframes blink {

  50% {
    opacity:.75;
  }

}


.hero h2 {

  margin:0;

}


.hero p {

  opacity:.85;

}


/* ========================================================
   CARDS
======================================================== */

.cards {

  display:grid;

  grid-template-columns:
    repeat(4,1fr);

  gap:18px;

}


.card {

  background:white;

  padding:22px;

  border-radius:14px;

  border:
    1px solid #e2e8f0;

  box-shadow:
    0 4px 15px
    rgba(0,0,0,.04);

}


.card-icon {

  font-size:30px;

}


.card-name {

  color:#64748b;

  margin-top:12px;

}


.card-value {

  font-size:27px;

  font-weight:bold;

  margin-top:5px;

}


.safe {

  color:#16a34a;

}


.warning {

  color:#f59e0b;

}


.danger {

  color:#dc2626;

}


/* ========================================================
   PANELS
======================================================== */

.panel {

  background:white;

  padding:22px;

  border-radius:14px;

  border:
    1px solid #e2e8f0;

  margin-top:20px;

}


.panel h2 {

  margin-top:0;

}


/* ========================================================
   TABLE
======================================================== */

table {

  width:100%;

  border-collapse:collapse;

}


th,td {

  padding:12px;

  border-bottom:
    1px solid #e2e8f0;

  text-align:left;

}


th {

  color:#64748b;

  font-size:12px;

}


/* ========================================================
   ANALYTICS
======================================================== */

.chart {

  width:100%;

  height:300px;

}


canvas {

  width:100%;

  height:100%;

}


/* ========================================================
   SYSTEM GRID
======================================================== */

.system-grid {

  display:grid;

  grid-template-columns:
    repeat(3,1fr);

  gap:18px;

}


.system-box {

  background:#f8fafc;

  padding:18px;

  border-radius:10px;

}


.system-box small {

  color:#64748b;

}


.system-box strong {

  display:block;

  margin-top:7px;

}


/* ========================================================
   ABOUT
======================================================== */

.about {

  background:white;

  padding:30px;

  border-radius:15px;

  line-height:1.7;

}


/* ========================================================
   MOBILE
======================================================== */

@media(max-width:950px) {

  .cards {

    grid-template-columns:
      repeat(2,1fr);

  }

  .system-grid {

    grid-template-columns:
      repeat(2,1fr);

  }

}


@media(max-width:650px) {

  .sidebar {

    width:65px;

    padding:10px;

  }


  .logo h2,
  .logo p,
  .menu-title,
  .nav span {

    display:none;

  }


  .logo-icon {

    font-size:25px;

  }


  .main {

    margin-left:65px;

  }


  .header {

    padding:0 15px;

  }


  .content {

    padding:15px;

  }


  .cards {

    grid-template-columns:1fr;

  }


  .system-grid {

    grid-template-columns:1fr;

  }

}

</style>

</head>


<body>


<!-- ======================================================
     SIDEBAR
======================================================= -->

<div class="sidebar">


<div class="logo">

<div class="logo-icon">
🏥
</div>

<h2>
Hospital IoT
</h2>

<p>
Emergency Safety System
</p>

</div>


<div class="menu-title">
Monitoring
</div>


<a
class="nav active"
onclick="showPage('dashboard',this)">

<span>🏠 Dashboard</span>

</a>


<a
class="nav"
onclick="showPage('monitoring',this)">

<span>📡 Live Monitoring</span>

</a>


<a
class="nav"
onclick="showPage('events',this)">

<span>🚨 Emergency Events</span>

</a>


<div class="menu-title">
Analysis
</div>


<a
class="nav"
onclick="showPage('analytics',this)">

<span>📈 Analytics</span>

</a>


<div class="menu-title">
System
</div>


<a
class="nav"
onclick="showPage('system',this)">

<span>⚙️ System Status</span>

</a>


<a
class="nav"
onclick="showPage('about',this)">

<span>ℹ️ About Project</span>

</a>


</div>



<!-- ======================================================
     MAIN
======================================================= -->

<div class="main">


<div class="header">


<div>

<h1>
Emergency Monitoring System
</h1>

<p>
ESP32 Local IoT Hospital Safety Network
</p>

</div>


<div
class="connection"
id="connection">

● Connected

</div>


</div>



<div class="content">


<!-- ====================================================
     DASHBOARD
===================================================== -->

<section
id="dashboard"
class="page active">


<div
class="hero"
id="hero">


<h2 id="overallStatus">
🟢 SYSTEM NORMAL
</h2>


<p id="overallMessage">

All monitored sensors are operating normally.

</p>


</div>



<div class="cards">


<div class="card">

<div class="card-icon">
🌡️
</div>

<div class="card-name">
Temperature
</div>

<div
id="temperature"
class="card-value">

-- °C

</div>

</div>



<div class="card">

<div class="card-icon">
🔥
</div>

<div class="card-name">
Fire
</div>

<div
id="fire"
class="card-value safe">

SAFE

</div>

</div>



<div class="card">

<div class="card-icon">
💨
</div>

<div class="card-name">
Smoke
</div>

<div
id="smoke"
class="card-value safe">

SAFE

</div>

</div>



<div class="card">

<div class="card-icon">
📳
</div>

<div class="card-name">
Vibration
</div>

<div
id="vibration"
class="card-value safe">

NORMAL

</div>

</div>


</div>


<div class="panel">

<h2>
System Overview
</h2>

<p>
The ESP32 continuously monitors environmental
and safety sensors and provides local
real-time status through this dashboard.
</p>

</div>


</section>



<!-- ====================================================
     LIVE MONITORING
===================================================== -->

<section
id="monitoring"
class="page">


<h2>
📡 Live Monitoring
</h2>


<div class="cards">


<div class="card">

<div class="card-icon">
🌡️
</div>

<div class="card-name">
Temperature
</div>

<div
id="monitorTemp"
class="card-value">

-- °C

</div>

<div>
DS18B20

</div>

</div>


<div class="card">

<div class="card-icon">
🔥
</div>

<div class="card-name">
Flame Sensor
</div>

<div
id="monitorFire"
class="card-value safe">

SAFE

</div>

</div>


<div class="card">

<div class="card-icon">
💨
</div>

<div class="card-name">
MQ-2 Smoke
</div>

<div
id="monitorSmoke"
class="card-value safe">

SAFE

</div>

<p>
Raw ADC:
<strong
id="smokeRaw">
0
</strong>
</p>

</div>


<div class="card">

<div class="card-icon">
📳
</div>

<div class="card-name">
Vibration
</div>

<div
id="monitorVibration"
class="card-value safe">

NORMAL

</div>

</div>


</div>


<div class="panel">

<h2>
Current Sensor Values
</h2>


<table>

<tr>

<th>
Sensor
</th>

<th>
Value
</th>

<th>
Status
</th>

</tr>


<tr>

<td>
Temperature
</td>

<td id="tableTemp">
--
</td>

<td id="tableTempStatus">
Normal
</td>

</tr>


<tr>

<td>
Flame
</td>

<td id="tableFire">
--
</td>

<td id="tableFireStatus">
Normal
</td>

</tr>


<tr>

<td>
Smoke
</td>

<td id="tableSmoke">
--
</td>

<td id="tableSmokeStatus">
Normal
</td>

</tr>


<tr>

<td>
Vibration
</td>

<td id="tableVibration">
--
</td>

<td id="tableVibrationStatus">
Normal
</td>

</tr>


</table>

</div>


</section>



<!-- ====================================================
     EVENTS
===================================================== -->

<section
id="events"
class="page">


<h2>
🚨 Emergency Events
</h2>


<div class="panel">


<table>

<thead>

<tr>

<th>
#
</th>

<th>
Time
</th>

<th>
Event
</th>

</tr>

</thead>


<tbody
id="eventTable">

<tr>

<td colspan="3">
No events recorded.
</td>

</tr>

</tbody>


</table>


</div>


</section>



<!-- ====================================================
     ANALYTICS
===================================================== -->

<section
id="analytics"
class="page">


<h2>
📈 Analytics
</h2>


<div class="panel">

<h2>
Temperature History
</h2>


<div class="chart">

<canvas
id="tempChart">
</canvas>

</div>


</div>



<div class="panel">

<h2>
Smoke Sensor History
</h2>


<div class="chart">

<canvas
id="smokeChart">
</canvas>

</div>


</div>


</section>



<!-- ====================================================
     SYSTEM
===================================================== -->

<section
id="system"
class="page">


<h2>
⚙️ System Status
</h2>


<div class="system-grid">


<div class="system-box">

<small>
ESP32
</small>

<strong>
Online
</strong>

</div>


<div class="system-box">

<small>
Wi-Fi
</small>

<strong
id="wifiStatus">

Connected

</strong>

</div>


<div class="system-box">

<small>
IP Address
</small>

<strong
id="deviceIP">

--

</strong>

</div>


<div class="system-box">

<small>
Temperature Sensor
</small>

<strong>
DS18B20

<span
id="tempSensorStatus">
Checking
</span>

</strong>

</div>


<div class="system-box">

<small>
Smoke Sensor
</small>

<strong>
MQ-2

<span>
Connected
</span>

</strong>

</div>


<div class="system-box">

<small>
Flame Sensor
</small>

<strong>
Connected
</strong>

</div>


<div class="system-box">

<small>
Vibration Sensor
</small>

<strong>
Connected
</strong>

</div>


<div class="system-box">

<small>
OLED Display
</small>

<strong>
SSD1306
</strong>

</div>


<div class="system-box">

<small>
Buzzer
</small>

<strong>
Available
</strong>

</div>


<div class="system-box">

<small>
System Uptime
</small>

<strong
id="uptime">

--

</strong>

</div>


<div class="system-box">

<small>
Last Update
</small>

<strong
id="lastUpdate">

--

</strong>

</div>


<div class="system-box">

<small>
Alarm State
</small>

<strong
id="alarmState">

NORMAL

</strong>

</div>


</div>


</section>



<!-- ====================================================
     ABOUT
===================================================== -->

<section
id="about"
class="page">


<h2>
ℹ️ About Project
</h2>


<div class="about">


<h2>
Hospital IoT Safety & Emergency Monitoring
</h2>


<p>

This project is an ESP32-based prototype
for monitoring environmental and safety
conditions in a hospital or laboratory
environment.

</p>


<h3>
Sensors
</h3>


<ul>

<li>
DS18B20 — Temperature monitoring
</li>

<li>
Flame sensor — Fire/flame detection
</li>

<li>
MQ-2 — Smoke/gas detection
</li>

<li>
Vibration sensor — Vibration detection
</li>

</ul>


<h3>
Controller

</h3>


<p>

ESP32 provides sensor processing,
local Wi-Fi connectivity, OLED output,
buzzer control and the web interface.

</p>


<h3>
Communication

</h3>


<p>

The ESP32 hosts the dashboard locally.
A phone or computer connected to the
same Wi-Fi network can open the ESP32
IP address to view sensor information.

</p>


<h3>
System Architecture
</h3>


<p>

Sensors → ESP32 → Wi-Fi → Web Dashboard

</p>


</div>


</section>


</div>


</div>



<script>


// ========================================================
// PAGE NAVIGATION
// ========================================================

function showPage(
  pageId,
  element
) {


  document
    .querySelectorAll(".page")
    .forEach(
      page => {

        page.classList.remove(
          "active"
        );

      }
    );


  document
    .getElementById(pageId)
    .classList.add(
      "active"
    );


  document
    .querySelectorAll(".nav")
    .forEach(
      nav => {

        nav.classList.remove(
          "active"
        );

      }
    );


  element.classList.add(
    "active"
  );

}



// ========================================================
// CHART DATA
// ========================================================

let temperatureHistory = [];

let smokeHistory = [];


// ========================================================
// DRAW CHART
// ========================================================

function drawChart(
  canvasId,
  data,
  color,
  maxValue
) {


  const canvas =
    document.getElementById(
      canvasId
    );


  if (!canvas) return;


  const width =
    canvas.parentElement
      .clientWidth;


  const height =
    canvas.parentElement
      .clientHeight;


  canvas.width =
    width;

  canvas.height =
    height;


  const ctx =
    canvas.getContext(
      "2d"
    );


  ctx.clearRect(
    0,
    0,
    width,
    height
  );


  // Grid

  ctx.strokeStyle =
    "#e2e8f0";

  ctx.lineWidth = 1;


  for (
    let i = 0;
    i <= 5;
    i++
  ) {


    const y =
      (height / 5) * i;


    ctx.beginPath();


    ctx.moveTo(
      0,
      y
    );


    ctx.lineTo(
      width,
      y
    );


    ctx.stroke();

  }


  if (
    data.length < 2
  ) return;


  // Line

  ctx.strokeStyle =
    color;

  ctx.lineWidth = 3;

  ctx.beginPath();


  data.forEach(
    (value,index) => {


      const x =
        index *
        width /
        (data.length - 1);


      const y =
        height -
        (
          value /
          maxValue
        ) *
        height;


      if (
        index === 0
      ) {

        ctx.moveTo(
          x,
          y
        );

      }

      else {

        ctx.lineTo(
          x,
          y
        );

      }

    }
  );


  ctx.stroke();

}



// ========================================================
// UPDATE EVENT TABLE
// ========================================================

function updateEvents(
  events
) {


  const table =
    document.getElementById(
      "eventTable"
    );


  if (
    !events ||
    events.length === 0
  ) {


    table.innerHTML = `

      <tr>

        <td colspan="3">

          No events recorded.

        </td>

      </tr>

    `;

    return;

  }


  table.innerHTML = "";


  events
    .slice()
    .reverse()
    .forEach(
      (event,index) => {


        const row =
          document.createElement(
            "tr"
          );


        row.innerHTML = `

          <td>
            ${index + 1}
          </td>

          <td>
            ${event.time}s
          </td>

          <td>
            ${event.message}
          </td>

        `;


        table.appendChild(
          row
        );

      }
    );

}



// ========================================================
// UPDATE WEBSITE
// ========================================================

async function updateData() {


  try {


    const response =
      await fetch(
        "/api/data"
      );


    if (!response.ok) {

      throw new Error(
        "API error"
      );

    }


    const data =
      await response.json();


    // ====================================================
    // CONNECTION
    // ====================================================

    document.getElementById(
      "connection"
    ).textContent =
      "● Connected";


    document.getElementById(
      "connection"
    ).style.color =
      "#16a34a";


    document.getElementById(
      "wifiStatus"
    ).textContent =
      "Connected";


    // ====================================================
    // TEMPERATURE
    // ====================================================

    const temp =
      Number(
        data.temperature
      );


    document.getElementById(
      "temperature"
    ).textContent =
      temp.toFixed(1)
      + " °C";


    document.getElementById(
      "monitorTemp"
    ).textContent =
      temp.toFixed(1)
      + " °C";


    document.getElementById(
      "tableTemp"
    ).textContent =
      temp.toFixed(1)
      + " °C";


    temperatureHistory.push(
      temp
    );


    if (
      temperatureHistory.length
      > 40
    ) {

      temperatureHistory.shift();

    }


    // ====================================================
    // FIRE
    // ====================================================

    const fire =
      document.getElementById(
        "fire"
      );


    const monitorFire =
      document.getElementById(
        "monitorFire"
      );


    if (
      data.fire
    ) {


      fire.textContent =
        "🔥 DETECTED";


      fire.className =
        "card-value danger";


      monitorFire.textContent =
        "🔥 DETECTED";


      monitorFire.className =
        "card-value danger";


      document.getElementById(
        "tableFire"
      ).textContent =
        "DETECTED";


      document.getElementById(
        "tableFireStatus"
      ).textContent =
        "ALARM";

    }

    else {


      fire.textContent =
        "SAFE";


      fire.className =
        "card-value safe";


      monitorFire.textContent =
        "SAFE";


      monitorFire.className =
        "card-value safe";


      document.getElementById(
        "tableFire"
      ).textContent =
        "SAFE";


      document.getElementById(
        "tableFireStatus"
      ).textContent =
        "Normal";

    }


    // ====================================================
    // SMOKE
    // ====================================================

    const smoke =
      document.getElementById(
        "smoke"
      );


    const monitorSmoke =
      document.getElementById(
        "monitorSmoke"
      );


    document.getElementById(
      "smokeRaw"
    ).textContent =
      data.smokeAnalog;


    smokeHistory.push(
      Number(
        data.smokeAnalog
      )
    );


    if (
      smokeHistory.length
      > 40
    ) {

      smokeHistory.shift();

    }


    if (
      data.smoke
    ) {


      smoke.textContent =
        "💨 DETECTED";


      smoke.className =
        "card-value danger";


      monitorSmoke.textContent =
        "💨 DETECTED";


      monitorSmoke.className =
        "card-value danger";


      document.getElementById(
        "tableSmoke"
      ).textContent =
        "DETECTED";


      document.getElementById(
        "tableSmokeStatus"
      ).textContent =
        "ALARM";

    }

    else {


      smoke.textContent =
        "SAFE";


      smoke.className =
        "card-value safe";


      monitorSmoke.textContent =
        "SAFE";


      monitorSmoke.className =
        "card-value safe";


      document.getElementById(
        "tableSmoke"
      ).textContent =
        "SAFE";


      document.getElementById(
        "tableSmokeStatus"
      ).textContent =
        "Normal";

    }


    // ====================================================
    // VIBRATION
    // ====================================================

    const vibration =
      document.getElementById(
        "vibration"
      );


    const monitorVibration =
      document.getElementById(
        "monitorVibration"
      );


    if (
      data.vibration
    ) {


      vibration.textContent =
        "📳 DETECTED";


      vibration.className =
        "card-value danger";


      monitorVibration.textContent =
        "📳 DETECTED";


      monitorVibration.className =
        "card-value danger";


      document.getElementById(
        "tableVibration"
      ).textContent =
        "DETECTED";


      document.getElementById(
        "tableVibrationStatus"
      ).textContent =
        "WARNING";

    }

    else {


      vibration.textContent =
        "NORMAL";


      vibration.className =
        "card-value safe";


      monitorVibration.textContent =
        "NORMAL";


      monitorVibration.className =
        "card-value safe";


      document.getElementById(
        "tableVibration"
      ).textContent =
        "NORMAL";


      document.getElementById(
        "tableVibrationStatus"
      ).textContent =
        "Normal";

    }


    // ====================================================
    // OVERALL STATUS
    // ====================================================

    const hero =
      document.getElementById(
        "hero"
      );


    const title =
      document.getElementById(
        "overallStatus"
      );


    const message =
      document.getElementById(
        "overallMessage"
      );


    if (
      data.alarm
    ) {


      hero.className =
        "hero alarm";


      title.textContent =
        "🚨 EMERGENCY DETECTED";


      message.textContent =
        "One or more sensors have detected an abnormal condition.";

    }

    else {


      hero.className =
        "hero";


      title.textContent =
        "🟢 SYSTEM NORMAL";


      message.textContent =
        "All monitored sensors are operating normally.";

    }


    // ====================================================
    // SYSTEM
    // ====================================================

    document.getElementById(
      "deviceIP"
    ).textContent =
      data.ip;


    document.getElementById(
      "alarmState"
    ).textContent =
      data.alarm
      ? "ALARM"
      : "NORMAL";


    document.getElementById(
      "lastUpdate"
    ).textContent =
      new Date()
      .toLocaleTimeString();


    // ====================================================
    // EVENTS
    // ====================================================

    updateEvents(
      data.events
    );


    // ====================================================
    // CHARTS
    // ====================================================

    drawChart(
      "tempChart",
      temperatureHistory,
      "#2563eb",
      60
    );


    drawChart(
      "smokeChart",
      smokeHistory,
      "#f97316",
      4095
    );


    // ====================================================
    // UPTIME
    // ====================================================

    const seconds =
      data.uptime;


    const hours =
      Math.floor(
        seconds / 3600
      );


    const minutes =
      Math.floor(
        (seconds % 3600) / 60
      );


    const secs =
      seconds % 60;


    document.getElementById(
      "uptime"
    ).textContent =
      hours + "h "
      +
      minutes + "m "
      +
      secs + "s";


  }


  catch(error) {


    console.log(
      error
    );


    document.getElementById(
      "connection"
    ).textContent =
      "● Disconnected";


    document.getElementById(
      "connection"
    ).style.color =
      "#dc2626";


    document.getElementById(
      "wifiStatus"
    ).textContent =
      "Disconnected";

  }

}


// ========================================================
// UPDATE EVERY SECOND
// ========================================================

setInterval(
  updateData,
  1000
);


updateData();


</script>


</body>

</html>

)rawliteral";


// ============================================================
// WEBSITE ROUTES
// ============================================================

void handleRoot() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


void handleMonitoring() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


void handleEvents() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


void handleAnalytics() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


void handleSystem() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


void handleAbout() {

  server.send(
    200,
    "text/html",
    webpage
  );
}


// ============================================================
// API
// ============================================================

void handleAPI() {

  readSensors();


  String json = "{";


  // Temperature

  json +=
    "\"temperature\":";

  json +=
    String(
      temperature,
      1
    );


  // Fire

  json +=
    ",\"fire\":";

  json +=
    fireDetected
    ? "true"
    : "false";


  // Smoke

  json +=
    ",\"smoke\":";

  json +=
    smokeDetected
    ? "true"
    : "false";


  // Smoke raw

  json +=
    ",\"smokeAnalog\":";

  json +=
    String(
      smokeAnalog
    );


  // Vibration

  json +=
    ",\"vibration\":";

  json +=
    vibrationDetected
    ? "true"
    : "false";


  // Alarm

  json +=
    ",\"alarm\":";

  json +=
    alarmActive
    ? "true"
    : "false";


  // IP

  json +=
    ",\"ip\":\"";

  json +=
    WiFi.localIP().toString();

  json +=
    "\"";


  // Uptime

  json +=
    ",\"uptime\":";

  json +=
    String(
      millis() / 1000
    );


  // Events

  json +=
    ",\"events\":[";


  for (
    int i = 0;
    i < eventCount;
    i++
  ) {


    if (i > 0) {

      json += ",";

    }


    json += "{";


    json +=
      "\"message\":\"";


    String safeMessage =
      eventMessages[i];


    safeMessage.replace(
      "\"",
      "\\\""
    );


    json +=
      safeMessage;


    json +=
      "\",\"time\":\"";


    json +=
      eventTimes[i];


    json +=
      "\"}";

  }


  json +=
    "]";


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(
    115200
  );


  delay(1000);


  // ========================================================
  // SENSOR PINS
  // ========================================================

  pinMode(
    FIRE_PIN,
    INPUT
  );


  pinMode(
    SMOKE_DO_PIN,
    INPUT
  );


  pinMode(
    VIBRATION_PIN,
    INPUT
  );


  // ========================================================
  // BUZZER
  // ========================================================

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );


  digitalWrite(
    BUZZER_PIN,
    LOW
  );


  // ========================================================
  // OLED
  // ========================================================

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  ) {


    Serial.println(
      "OLED ERROR"
    );


    while (true) {

      delay(1000);

    }

  }


  // ========================================================
  // TEMPERATURE
  // ========================================================

  tempSensor.begin();


  // ========================================================
  // START OLED
  // ========================================================

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(
    0,
    0
  );


  display.println(
    "HOSPITAL IoT"
  );


  display.println();


  display.println(
    "Connecting WiFi..."
  );


  display.display();


  // ========================================================
  // WIFI
  // ========================================================

  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.print(
    "Connecting WiFi"
  );


  int attempts = 0;


  while (
    WiFi.status()
    != WL_CONNECTED
    &&
    attempts < 40
  ) {


    delay(500);

    Serial.print(
      "."
    );


    attempts++;

  }


  Serial.println();


  // ========================================================
  // WIFI SUCCESS
  // ========================================================

  if (
    WiFi.status()
    == WL_CONNECTED
  ) {


    Serial.println(
      "WiFi connected!"
    );


    Serial.print(
      "ESP32 IP: "
    );


    Serial.println(
      WiFi.localIP()
    );


    display.clearDisplay();

    display.setCursor(
      0,
      0
    );


    display.println(
      "WIFI CONNECTED"
    );


    display.println();


    display.println(
      WiFi.localIP()
    );


    display.display();

  }


  else {


    Serial.println(
      "WiFi connection failed"
    );


    display.clearDisplay();

    display.setCursor(
      0,
      0
    );


    display.println(
      "WIFI FAILED"
    );


    display.display();

  }


  // ========================================================
  // WEB ROUTES
  // ========================================================

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/monitoring",
    handleMonitoring
  );


  server.on(
    "/events",
    handleEvents
  );


  server.on(
    "/analytics",
    handleAnalytics
  );


  server.on(
    "/system",
    handleSystem
  );


  server.on(
    "/about",
    handleAbout
  );


  server.on(
    "/api/data",
    handleAPI
  );


  // ========================================================
  // START SERVER
  // ========================================================

  server.begin();


  Serial.println(
    "Web server started"
  );


  // ========================================================
  // SYSTEM TIMER
  // ========================================================

  systemStartTime =
    millis();


  addEvent(
    "System started"
  );


  delay(1000);
}


// ============================================================
// LOOP
// ============================================================

void loop() {


  // Sensor reading

  if (
    millis() -
    lastSensorRead
    >= 500
  ) {


    lastSensorRead =
      millis();


    readSensors();

  }


  // OLED

  if (
    millis() -
    lastOLEDUpdate
    >= 500
  ) {


    lastOLEDUpdate =
      millis();


    updateOLED();

  }


  // Buzzer

  updateBuzzer();


  // Web server

  server.handleClient();


  delay(5);
}
