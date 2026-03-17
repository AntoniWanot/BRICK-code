#ifndef MY_WEBSERVER_H
#define MY_WEBSERVER_H

#include <ESPAsyncWebServer.h>

// Function declarations
void setupWebServer();
void handleRoot(AsyncWebServerRequest *request);
void handleStart(AsyncWebServerRequest *request);
void handleJogPlus(AsyncWebServerRequest *request);
void handleJogMinus(AsyncWebServerRequest *request);
void handleStopJog(AsyncWebServerRequest *request);
void checkJogTimeout(); // Funkcja automatycznie wyłączająca silniki po pulsie
void checkStartTimeout(); // Funkcja automatycznie wyłączająca START_PIN po pulsie
String getMainPage();

// Global variables
extern AsyncWebServer server;
extern int selectedMotor; // 1 or 2

#endif
