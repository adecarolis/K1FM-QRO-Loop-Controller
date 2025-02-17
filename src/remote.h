#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include "memories.h"
#include "stepper.h"
#include "lcd.h"
#include "debug.h"

#ifdef EMULATOR
// These credentials are for the Wokwi emulator
// do not change them
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
// Defining the WiFi channel speeds up the connection:
#define WIFI_CHANNEL 6
#else
// Set your WIFI credentials here
#define WIFI_SSID "MyWIFI"
#define WIFI_PASSWORD "MyWIFIpassword"
#endif

extern WebServer server;
void setupWebServer(); 
void remote_loop();