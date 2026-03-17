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
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Robot Brick Controller</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }";
  html += ".container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += ".tabs { display: flex; border-bottom: 2px solid #ddd; margin-bottom: 20px; }";
  html += ".tab { padding: 10px 20px; background-color: #f0f0f0; border: none; cursor: pointer; font-size: 16px; margin-right: 5px; border-radius: 5px 5px 0 0; }";
  html += ".tab.active { background-color: #007bff; color: white; }";
  html += ".tab-content { display: none; text-align: center; }";
  html += ".tab-content.active { display: block; }";
  html += ".start-button { background-color: #28a745; color: white; border: none; padding: 15px 30px; font-size: 18px; border-radius: 5px; cursor: pointer; margin: 20px 0; }";
  html += ".start-button:hover { background-color: #218838; }";
  html += ".program-label { font-size: 16px; color: #666; margin-top: 10px; }";
  html += ".motor-selector { margin: 20px 0; }";
  html += ".motor-button { background-color: #6c757d; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 5px; cursor: pointer; margin: 5px; }";
  html += ".motor-button.selected { background-color: #007bff; }";
  html += ".motor-button:hover { background-color: #5a6268; }";
  html += ".motor-button.selected:hover { background-color: #0056b3; }";
  html += ".jog-controls { margin: 30px 0; }";
  html += ".jog-button { background-color: #17a2b8; color: white; border: none; padding: 20px 40px; font-size: 20px; border-radius: 10px; cursor: pointer; margin: 10px; min-width: 120px; }";
  html += ".jog-button:hover { background-color: #138496; }";
  html += ".jog-plus { background-color: #28a745; }";
  html += ".jog-plus:hover { background-color: #218838; }";
  html += ".jog-minus { background-color: #dc3545; }";
  html += ".jog-minus:hover { background-color: #c82333; }";
  html += ".status { margin: 20px 0; padding: 10px; background-color: #f8f9fa; border-radius: 5px; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>🤖 Robot Brick Controller</h1>";
  html += "<div class='tabs'>";
  html += "<button class='tab active' onclick='openTab(event, \"main-tab\")'>Program</button>";
  html += "<button class='tab' onclick='openTab(event, \"control-tab\")'>Sterowanie</button>";
  html += "</div>";
  html += "<div id='main-tab' class='tab-content active'>";
  html += "<h2>Program Testowy</h2>";
  html += "<button class='start-button' onclick='startProgram()'>START</button>";
  html += "<div class='program-label'>program_testowy</div>";
  html += "<div id='status'></div>";
  html += "</div>";
  html += "<div id='control-tab' class='tab-content'>";
  html += "<h2>Sterowanie Silnikami</h2>";
  html += "<div class='motor-selector'>";
  html += "<h3>Wybierz Silnik:</h3>";
  html += "<button id='motor1-btn' class='motor-button selected' onclick='selectMotor(1)'>Silnik 1</button>";
  html += "<button id='motor2-btn' class='motor-button' onclick='selectMotor(2)'>Silnik 2</button>";
  html += "</div>";
  html += "<div class='jog-controls'>";
  html += "<h3>Sterowanie JOG:</h3>";
  html += "<div>";
  html += "<button class='jog-button jog-plus' onpointerdown='startJog("plus")' onpointerup='stopJog()' onpointercancel='stopJog()' onpointerleave='stopJog()'>JOG +</button>";
  html += "<button class='jog-button jog-minus' onpointerdown='startJog("minus")' onpointerup='stopJog()' onpointercancel='stopJog()' onpointerleave='stopJog()'>JOG -</button>";
  html += "</div>";
  html += "</div>";
  html += "<div class='status' id='motor-status'>Wybrany: Silnik 1 | JOG: Zatrzymany</div>";
  html += "</div>";
  html += "</div>";
  html += "<script>";
  html += "var selectedMotor = 1;";
  html += "var jogInterval;";
  html += "var currentJog = '';";
  html += "function openTab(evt, tabName) {";
  html += "var i, tabcontent, tablinks;";
  html += "tabcontent = document.getElementsByClassName('tab-content');";
  html += "for (i = 0; i < tabcontent.length; i++) {";
  html += "tabcontent[i].classList.remove('active');";
  html += "}";
  html += "tablinks = document.getElementsByClassName('tab');";
  html += "for (i = 0; i < tablinks.length; i++) {";
  html += "tablinks[i].classList.remove('active');";
  html += "}";
  html += "document.getElementById(tabName).classList.add('active');";
  html += "evt.currentTarget.classList.add('active');";
  html += "}";
  html += "function startProgram() {";
  html += "fetch('/start', { method: 'POST' })";
  html += ".then(response => response.text())";
  html += ".then(data => {";
  html += "document.getElementById('status').innerHTML = '<p style=\"color: green;\">' + data + '</p>';";
  html += "})";
  html += ".catch(error => {";
  html += "document.getElementById('status').innerHTML = '<p style=\"color: red;\">Error: ' + error + '</p>';";
  html += "});";
  html += "}";
  html += "function selectMotor(motor) {";
  html += "selectedMotor = motor;";
  html += "document.getElementById('motor1-btn').classList.remove('selected');";
  html += "document.getElementById('motor2-btn').classList.remove('selected');";
  html += "document.getElementById('motor' + motor + '-btn').classList.add('selected');";
  html += "fetch('/select_motor' + motor, { method: 'POST' });";
  html += "updateStatus();";
  html += "}";
  html += "function startJog(direction) {";
  html += "currentJog = direction;";
  html += "var endpoint = direction === 'plus' ? '/jog_plus' : '/jog_minus';";
  html += "var ts = Date.now();";
  html += "fetch(endpoint+'?ts=' + ts, { method: 'POST' });";
  html += "updateStatus();";
  html += "}";
  html += "function stopJog() {";
  html += "var ts = Date.now();";
  html += "fetch('/stop_jog?ts=' + ts, { method: 'POST' });";
  html += "currentJog = '';";
  html += "updateStatus();";
  html += "}";
  html += "function updateStatus() {";
  html += "var jogStatus = currentJog ? 'JOG ' + (currentJog === 'plus' ? '+' : '-') + ' AKTYWNY' : 'Zatrzymany';";
  html += "document.getElementById('motor-status').textContent = 'Wybrany: Silnik ' + selectedMotor + ' | JOG: ' + jogStatus;";
  html += "}";
  html += "</script>";
  html += "</body>";
  html += "</html>";
  return html;
}
