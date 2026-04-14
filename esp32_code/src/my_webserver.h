#ifndef MY_WEBSERVER_H
#define MY_WEBSERVER_H

#include <ESPAsyncWebServer.h>

// Function declarations
void setupWebServer();
void setCachedManifest(const String &manifest);
void handleRoot(AsyncWebServerRequest *request);
void handleStart(AsyncWebServerRequest *request);
void handleJogPlus(AsyncWebServerRequest *request);
void handleJogMinus(AsyncWebServerRequest *request);
void handleStopJog(AsyncWebServerRequest *request);
void checkJogTimeout(); 
void checkStartTimeout(); 
String getMainPage();

// Global variables
extern AsyncWebServer server;
extern int selectedMotor; 

#endif
