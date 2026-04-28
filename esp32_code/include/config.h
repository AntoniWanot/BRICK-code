#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID "ESP32_WebServer"
#define WIFI_PASSWORD "12345678"
#define READY_SIGNAL 22 // signal from esp32 to teensy that it is ready to receive manifest, 13 teensy
#define MOTOR1_PLUS_PIN 2 // motor 1 JOG+ 36 teensy
#define MOTOR1_MINUS_PIN 4// motor 1 JOG- 37 teensy
#define MOTOR2_PLUS_PIN 5 // motor 2 JOG+ 38 teensy
#define MOTOR2_MINUS_PIN 18// motor 2 JOG-  39 teensy
#define MOTOR3_PLUS_PIN 19 // motor 3 JOG+ 40 teensy
#define MOTOR3_MINUS_PIN 21// motor 3 JOG- 41 teensy
#define RXD2 16
#define TXD2 17
#endif
