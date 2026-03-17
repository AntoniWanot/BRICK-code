#include "my_webserver.h"
#include "config.h"
#include <WiFi.h>
#include <SPIFFS.h>

AsyncWebServer server(80);
int selectedMotor = 1; // deafult motor is 1
unsigned long lastJogSeq = 0;

// autostop parameters
unsigned long lastJogTime = 0;
const unsigned long JOG_PULSE_TIME = 150; // 150ms puls HIGH, than LOW

// automatic bump start parameters
unsigned long startPinActivationTime = 0;
const unsigned long START_PULSE_TIME = 200; // 200ms puls HIGH, than LOW

void setupWebServer() {
  // Initialize SPIFFS to serve static files (test.html)
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed - static files may be unavailable");
  }
  // Serve main page
  server.on("/", HTTP_GET, handleRoot);
  
  // Handle start button
  server.on("/start", HTTP_POST, handleStart);
  
  // Handle motor selection
  server.on("/select_motor1", HTTP_POST, [](AsyncWebServerRequest *request){
    selectedMotor = 1;
    Serial.println("Selected Motor 1");
    request->send(200, "text/plain", "Motor 1 selected");
  });
  
  server.on("/select_motor2", HTTP_POST, [](AsyncWebServerRequest *request){
    selectedMotor = 2;
    Serial.println("Selected Motor 2");
    request->send(200, "text/plain", "Motor 2 selected");
  });
  
  // Handle jog controls
  server.on("/jog_plus", HTTP_POST, handleJogPlus);
  server.on("/jog_minus", HTTP_POST, handleJogMinus);
  server.on("/stop_jog", HTTP_POST, handleStopJog);
  
  // Endpoint: return manifest JSON received from the serial-connected device
  server.on("/manifest", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial2.println("READY");
    unsigned long start_time = millis();
    String response;
    while (millis() - start_time < 5000) {
      while (Serial2.available()) {
        response += (char)Serial2.read();
        if (response.indexOf("END_OF_MANIFEST") != -1) break;
      }
      if (response.indexOf("END_OF_MANIFEST") != -1) break;
      delay(5);
    }
    if (response.length() == 0) {
      request->send(504, "text/plain", "Timeout waiting for manifest");
      return;
    }
    int idx = response.indexOf("END_OF_MANIFEST");
    if (idx != -1) response = response.substring(0, idx);
    response.trim();
    // Send as application/json (assume sender provided valid JSON)
    request->send(200, "application/json", response);
  });

  // Endpoint: select program by id (POST with query param ?id=)
  server.on("/select_program", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!request->hasParam("id")) {
      request->send(400, "text/plain", "Missing id param");
      return;
    }
    int id = request->getParam("id")->value().toInt();
    Serial.print("Sending program id to device: ");
    Serial.println(id);
    Serial2.println(id);
    request->send(200, "text/plain", "Program selected");
  });

  server.begin();
}

void handleRoot(AsyncWebServerRequest *request) {
  // Try to serve the static HTML from SPIFFS first
  if (SPIFFS.exists("/test.html")) {
    request->send(SPIFFS, "/test.html", "text/html");
    return;
  }
  // Fallback to embedded page generator
  request->send(200, "text/html", getMainPage());
}

void handleStart(AsyncWebServerRequest *request) {
  Serial.println("Start button pressed!");
  
  digitalWrite(START_PIN, HIGH);
  startPinActivationTime = millis();
  Serial.println("START_PIN set to HIGH");
  
  request->send(200, "text/plain", "Program testowy started! START_PIN = HIGH");
}

void handleJogPlus(AsyncWebServerRequest *request) {
  unsigned long seq = 0;
  if (request->hasParam("ts")) {
    seq = (unsigned long) request->getParam("ts")->value().toInt();
  }
  if (seq < lastJogSeq) {
    request->send(200, "text/plain", "stale");
    return;
  }
  lastJogSeq = seq;
  lastJogTime = millis(); // Save time of last JOG signal

  if (selectedMotor == 1) {
    digitalWrite(MOTOR1_PLUS_PIN, HIGH);
    digitalWrite(MOTOR1_MINUS_PIN, LOW);
    Serial.println("Motor 1 JOG+ activated");
    request->send(200, "text/plain", "Motor 1 JOG+ ON");
  } 
  else if (selectedMotor == 2) {
    digitalWrite(MOTOR2_PLUS_PIN, HIGH);
    digitalWrite(MOTOR2_MINUS_PIN, LOW);
    Serial.println("Motor 2 JOG+ activated");
    request->send(200, "text/plain", "Motor 2 JOG+ ON");
  }
  else {
    digitalWrite(MOTOR3_PLUS_PIN, HIGH);
    digitalWrite(MOTOR3_MINUS_PIN, LOW);
    Serial.println("Motor 3 JOG+ activated");
    request->send(200, "text/plain", "Motor 3 JOG+ ON");
  }
}

void handleJogMinus(AsyncWebServerRequest *request) {
  unsigned long seq = 0;
  if (request->hasParam("ts")) {
    seq = (unsigned long) request->getParam("ts")->value().toInt();
  }
  if (seq < lastJogSeq) {
    request->send(200, "text/plain", "stale");
    return;
  }
  lastJogSeq = seq;
  lastJogTime = millis(); // Save time of last JOG signal
  lastJogTime = millis(); // Save time of last JOG signal
  if (selectedMotor == 1) {
    digitalWrite(MOTOR1_PLUS_PIN, LOW);
    digitalWrite(MOTOR1_MINUS_PIN, HIGH);
    Serial.println("Motor 1 JOG- activated");
    request->send(200, "text/plain", "Motor 1 JOG- ON");
  } else if (selectedMotor == 2) {
    digitalWrite(MOTOR2_PLUS_PIN, LOW);
    digitalWrite(MOTOR2_MINUS_PIN, HIGH);
    Serial.println("Motor 2 JOG- activated");
    request->send(200, "text/plain", "Motor 2 JOG- ON");
  }
  else {
    digitalWrite(MOTOR3_PLUS_PIN, LOW);
    digitalWrite(MOTOR3_MINUS_PIN, HIGH);
    Serial.println("Motor 3 JOG- activated");
    request->send(200, "text/plain", "Motor 3 JOG- ON");
  }
}

void handleStopJog(AsyncWebServerRequest *request) {
  unsigned long seq = 0;
  if (request->hasParam("ts")) {
    seq = (unsigned long) request->getParam("ts")->value().toInt();
  }
  if (seq < lastJogSeq) {
    request->send(200, "text/plain", "stale-stop");
    return;
  }
  lastJogSeq = seq;

  // Stop all motors regardless of selection
  digitalWrite(MOTOR1_PLUS_PIN, LOW);
  digitalWrite(MOTOR1_MINUS_PIN, LOW);
  digitalWrite(MOTOR2_PLUS_PIN, LOW);
  digitalWrite(MOTOR2_MINUS_PIN, LOW);
  digitalWrite(MOTOR3_PLUS_PIN, LOW);
  digitalWrite(MOTOR3_MINUS_PIN, LOW);
  lastJogTime = 0; // cancel pulse timeout
  Serial.println("All motors stopped");
  request->send(200, "text/plain", "JOG stopped");
}

// automatic function to stop JOG signal after defined time
void checkJogTimeout() {
  if (lastJogTime > 0 && (millis() - lastJogTime > JOG_PULSE_TIME)) {
    digitalWrite(MOTOR1_PLUS_PIN, LOW);
    digitalWrite(MOTOR1_MINUS_PIN, LOW);
    digitalWrite(MOTOR2_PLUS_PIN, LOW);
    digitalWrite(MOTOR2_MINUS_PIN, LOW);
    lastJogTime = 0; 
    Serial.println("JOG pulse ended - motors stopped");
  }
}

// automatic function to stop START_PIN after defined time
void checkStartTimeout() {
  if (startPinActivationTime > 0 && (millis() - startPinActivationTime > START_PULSE_TIME)) {
    // After 200ms from activation - deactivate START_PIN
    digitalWrite(START_PIN, LOW);
    startPinActivationTime = 0; // Reset time
    Serial.println("START pulse ended - START_PIN set to LOW");
  }
}

String getMainPage() {
  String html = R"HTML(<!DOCTYPE html>
<html>
<head>
  <title>Robot Brick Controller</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }
    .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
    .tabs { display: flex; border-bottom: 2px solid #ddd; margin-bottom: 20px; }
    .tab { padding: 10px 20px; background-color: #f0f0f0; border: none; cursor: pointer; font-size: 16px; margin-right: 5px; border-radius: 5px 5px 0 0; }
    .tab.active { background-color: #007bff; color: white; }
    .tab-content { display: none; text-align: center; }
    .tab-content.active { display: block; }
    .start-button { background-color: #28a745; color: white; border: none; padding: 15px 30px; font-size: 18px; border-radius: 5px; cursor: pointer; margin: 20px 0; }
    .start-button:hover { background-color: #218838; }
    .program-label { font-size: 16px; color: #666; margin-top: 10px; }
    .motor-selector { margin: 20px 0; }
    .motor-button { background-color: #6c757d; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; margin: 5px; }
    .motor-button.selected { background-color: #007bff; }
    .motor-button:hover { background-color: #5a6268; }
    .motor-button.selected:hover { background-color: #0056b3; }
    .jog-controls { margin: 30px 0; }
    .jog-button { background-color: #17a2b8; color: white; border: none; padding: 20px 40px; font-size: 20px; border-radius: 10px; cursor: pointer; margin: 10px; min-width: 120px; }
    .jog-button:hover { background-color: #138496; }
    .jog-plus { background-color: #28a745; }
    .jog-plus:hover { background-color: #218838; }
    .jog-minus { background-color: #dc3545; }
    .jog-minus:hover { background-color: #c82333; }
    .status { margin: 20px 0; padding: 10px; background-color: #f8f9fa; border-radius: 5px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Robot Brick Controller</h1>
    <div class="tabs">
      <button class="tab active" onclick="openTab(event, 'main-tab')">Program</button>
      <button class="tab" onclick="openTab(event, 'control-tab')">Sterowanie</button>
    </div>

    <div id="main-tab" class="tab-content active">
      <h2>Available Programs</h2>
        <div id="program-list">Loading programs...</div>
        <button class="start-button" onclick="startProgram()">START</button>
        <div class="program-label" id="selected-program">No program selected</div>
        <div id="status"></div>
    </div>

    <div id="control-tab" class="tab-content">
      <h2>Motor jogging</h2>
      <div class="motor-selector">
        <h3>Choose Motor:</h3>
        <button id="motor1-btn" class="motor-button selected" onclick="selectMotor(1)">Motor 1</button>
        <button id="motor2-btn" class="motor-button" onclick="selectMotor(2)">Motor 2</button>
        <button id="motor3-btn" class="motor-button" onclick="selectMotor(3)">Motor 3</button>
      </div>
      <div class="jog-controls">
        <h3>JOG Control:</h3>
        <div>
          <button class="jog-button jog-plus" onpointerdown="startJog('plus')" onpointerup="stopJog()" onpointercancel="stopJog()" onpointerleave="stopJog()">JOG +</button>
          <button class="jog-button jog-minus" onpointerdown="startJog('minus')" onpointerup="stopJog()" onpointercancel="stopJog()" onpointerleave="stopJog()">JOG -</button>
        </div>
      </div>
      <div class="status" id="motor-status">Chosen: Motor 1 | JOG: Stopped</div>
    </div>
  </div>

  <script>
    var selectedMotor = 1;
    var currentJog = '';

    function openTab(evt, tabName) {
      var i, tabcontent, tablinks;
      tabcontent = document.getElementsByClassName('tab-content');
      for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].classList.remove('active');
      }
      tablinks = document.getElementsByClassName('tab');
      for (i = 0; i < tablinks.length; i++) {
        tablinks[i].classList.remove('active');
      }
      document.getElementById(tabName).classList.add('active');
      evt.currentTarget.classList.add('active');
    }

    let programs = [];
    let selectedProgramId = null;

    function loadManifest() {
      fetch('/manifest')
      .then(resp => {
        if (!resp.ok) throw new Error('Failed to fetch manifest: ' + resp.status);
        return resp.json();
      })
      .then(data => {
        programs = data.programs || [];
        renderProgramList();
      })
      .catch(err => {
        document.getElementById('program-list').textContent = 'Error loading manifest: ' + err;
      });
    }

    // Render program list as a table so radios align vertically
    function renderProgramList() {
      const container = document.getElementById('program-list');
      container.innerHTML = '';
      if (!programs.length) {
        container.textContent = 'No programs found';
        return;
      }
      const table = document.createElement('table');
      table.className = 'program-list-table';
      table.style.width = '100%';
      table.style.borderCollapse = 'collapse';

      const tbody = document.createElement('tbody');
      programs.forEach(p => {
        const tr = document.createElement('tr');
        tr.style.borderBottom = '1px solid #eee';

        const tdRadio = document.createElement('td');
        tdRadio.style.padding = '8px';
        tdRadio.style.width = '40px';
        tdRadio.style.verticalAlign = 'top';
        const radio = document.createElement('input');
        radio.type = 'radio';
        radio.name = 'program';
        radio.value = p.id;
        radio.id = 'prog-' + p.id;
        radio.onclick = () => selectProgram(p.id);
        tdRadio.appendChild(radio);
        tr.appendChild(tdRadio);

        const tdInfo = document.createElement('td');
        tdInfo.style.padding = '8px';
        tdInfo.style.verticalAlign = 'top';
        const title = document.createElement('div');
        title.innerHTML = '<strong>' + escapeHtml(p.name) + '</strong>';
        const desc = document.createElement('div');
        desc.style.color = '#666';
        desc.textContent = p.description || '';
        const meta = document.createElement('div');
        meta.style.color = '#888';
        meta.style.fontSize = '12px';
        meta.textContent = p.created || '';
        tdInfo.appendChild(title);
        tdInfo.appendChild(desc);
        tdInfo.appendChild(meta);
        tr.appendChild(tdInfo);

        tbody.appendChild(tr);
      });

      table.appendChild(tbody);
      container.appendChild(table);
    }

    function selectProgram(id) {
      selectedProgramId = id;
      const p = programs.find(x => x.id == id) || {};
      document.getElementById('selected-program').textContent = p.name ? p.name + ' (ID ' + id + ')' : 'ID ' + id;
    }

    function startProgram() {
      if (!selectedProgramId) {
        document.getElementById('status').innerHTML = '<p style="color: red;">Select a program first</p>';
        return;
      }
      fetch('/select_program?id=' + encodeURIComponent(selectedProgramId), { method: 'POST' })
      .then(resp => resp.text())
      .then(() => fetch('/start', { method: 'POST' }))
      .then(resp => resp.text())
      .then(data => {
        document.getElementById('status').innerHTML = '<p style="color: green;">' + data + '</p>';
      })
      .catch(error => {
        document.getElementById('status').innerHTML = '<p style="color: red;">Error: ' + error + '</p>';
      });
    }

    function escapeHtml(s) { return String(s).replace(/[&<>\"']/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','\"':'&quot;',"'":"&#39;"}[c]; }); }

    function selectMotor(motor) {
      selectedMotor = motor;
      document.getElementById('motor1-btn').classList.remove('selected');
      document.getElementById('motor2-btn').classList.remove('selected');
      document.getElementById('motor3-btn').classList.remove('selected');
      document.getElementById('motor' + motor + '-btn').classList.add('selected');
      fetch('/select_motor' + motor, { method: 'POST' });
      updateStatus();
    }

    function startJog(direction) {
      currentJog = direction;
      var endpoint = direction === 'plus' ? '/jog_plus' : '/jog_minus';
      var ts = Date.now();
      fetch(endpoint + '?ts=' + ts, { method: 'POST' });
      updateStatus();
    }

    function stopJog() {
      var ts = Date.now();
      fetch('/stop_jog?ts=' + ts, { method: 'POST' });
      currentJog = '';
      updateStatus();
    }

    function updateStatus() {
      var jogStatus = currentJog ? 'JOG ' + (currentJog === 'plus' ? '+' : '-') + ' ACTIVE' : 'STOPPED';
      document.getElementById('motor-status').textContent = 'Chosen: Motor ' + selectedMotor + ' | JOG: ' + jogStatus;
    }

    // Load manifest when the page is ready
    window.addEventListener('load', loadManifest);
  </script>
</body>
</html>)HTML";

  return html;
}
