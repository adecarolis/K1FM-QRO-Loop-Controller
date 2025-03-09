#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <Preferences.h>
#include "memories.h"
#include "stepper.h"
#include "lcd.h"
#include "debug.h"

#define SSID_KEY "ssid"
#define PASSWORD_KEY "password"
#define RIGCTLD_HOST_KEY "rigctl_host"
#define RIGCTLD_PORT_KEY "rigctl_port"

#ifdef EMULATOR
// These credentials are for the Wokwi emulator
// do not change them
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
// Defining the WiFi channel speeds up the connection:
#define WIFI_CHANNEL 6
#endif

extern WebServer server;
extern Preferences preferences;
void setupWebServer(); 
void remote_loop();
void handleWifiConfig();
void handleSetupRigctld();