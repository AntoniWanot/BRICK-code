#include <Arduino.h>
#include <SoftwareSerial.h>
// Teensy 4.1 - prosty generator STEP/DIR (test)
const int STEP_PIN = 2;   
const int DIR_PIN  = 3;

volatile bool stepState = false;
IntervalTimer stepTimer;

void stepISR() {
  stepState = !stepState;
  digitalWriteFast(STEP_PIN, stepState); 
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH); 

  // ustaw częstotliwość: np. 1000 Hz -> period 1 ms -> dla 50% duty call ISR co 0.5ms
  float freqHz = 5000.0;        
  float halfPeriodUs = 1e6f / (freqHz * 2.0f); 
  stepTimer.begin(stepISR, (int)halfPeriodUs); 
}

void loop() {
  // możesz zmieniać kierunek co n sekund
  delay(3000);
  digitalWrite(DIR_PIN, !digitalRead(DIR_PIN));
}
