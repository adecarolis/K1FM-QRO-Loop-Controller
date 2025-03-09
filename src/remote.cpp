#include "remote.h"
#include "remote.html.h"
#include "version.h"
#include <Preferences.h>
#include "automation.h"

WebServer server(80);

String getHtmlContent() {
    String htmlContent = htmlTemplate;
    int pos = htmlContent.indexOf("{{VERSION}}");
    if (pos != -1) {
        htmlContent.replace("{{VERSION}}", VERSION);
    }
    pos = htmlContent.indexOf("{{AP_MODE}}");
    if (pos != -1) {
        htmlContent.replace("{{AP_MODE}}", WiFi.getMode() == WIFI_MODE_AP ? "true" : "false");
    }
    pos = htmlContent.indexOf("{{AP_MODE_SETUP}}");
    if (pos != -1) {
        if (WiFi.getMode() == WIFI_MODE_AP) {
            String savedSsid = preferences.getString(SSID_KEY, "");
            String savedPassword = preferences.getString(PASSWORD_KEY, "");
            String savedRigctld_host = preferences.getString(RIGCTLD_HOST_KEY, "");
            u_int16_t savedRigctld_port = preferences.getUInt(RIGCTLD_PORT_KEY, 4532);
            htmlContent.replace("{{AP_MODE_SETUP}}", R"rawliteral(
              <div class="section" id="apModeSetup">
                <h2>WiFi Setup</h2>
                <form action="/setup_wifi" method="post" style="display: flex; flex-direction: column; gap: 10px;">
                  <label for="ssid" style="font-weight: bold;">SSID:</label>
                  <input type="text" id="ssid" name="ssid" value=")rawliteral" + savedSsid + R"rawliteral(" required style="padding: 8px; border-radius: 5px; border: 1px solid #ccc;">
                  <label for="password" style="font-weight: bold;">Password:</label>
                  <input type="password" id="password" name="password" value=")rawliteral" + savedPassword + R"rawliteral(" required style="padding: 8px; border-radius: 5px; border: 1px solid #ccc;">
                  <label for="rigctld_host" style="font-weight: bold;">Rigctld Host:</label>
                  <input type="text" id="rigctld_host" name="rigctld_host" value=")rawliteral" + savedRigctld_host + R"rawliteral(" required style="padding: 8px; border-radius: 5px; border: 1px solid #ccc;">
                  <label for="rigctld_port" style="font-weight: bold;">Rigctld Port:</label>
                  <input type="text" id="rigctld_port" name="rigctld_port" value=")rawliteral" + savedRigctld_port + R"rawliteral(" required style="padding: 8px; border-radius: 5px; border: 1px solid #ccc;">
                  <button type="submit" class="btn save-button" style="align-self: flex-start;">Connect</button>
                </form>
              </div>
            )rawliteral");
            htmlContent.replace("{{DISCONNECT_LINK}}", ""); // Remove disconnect link in AP mode
        } else {
            htmlContent.replace("{{AP_MODE_SETUP}}", "");
            htmlContent.replace("{{DISCONNECT_LINK}}", R"rawliteral(
              <a href="javascript:disconnectWifi()">Disconnect WiFi</a>
            )rawliteral");
        }
    }
    pos = htmlContent.indexOf("{{EMULATOR_WARNING}}");
    #ifdef EMULATOR
    if (pos != -1) {
        htmlContent.replace("{{EMULATOR_WARNING}}", R"rawliteral(
          <div class="emulator-warning">
            WARNING: EMULATOR mode is enabled. DO NOT USE.
          </div>
        )rawliteral");
    }
    #else
    if (pos != -1) {
        htmlContent.replace("{{EMULATOR_WARNING}}", "");
    }
    #endif

    // Replace {{ENDSTOP_KEY}} with the actual endstop value
    String currentEndstop = preferences.getString(ENDSTOP_KEY, "");
    htmlContent.replace("{{ENDSTOP_KEY}}", currentEndstop);

    return htmlContent;
}

void sendHtml() {
  server.send(200, "text/html", getHtmlContent());
}

void handleAdjustCommand() {
  String command = server.pathArg(0);

  if (command == "set") {
    uint16_t result = adjustingSet();
    server.send(200, "text/plain", String(result));
  } else if (command == "unset") {
    uint16_t result = adjustingUnset();
    server.send(200, "text/plain", String(result));
  } else if (command == "status") {
    bool status = adjusting;
    server.send(200, "text/plain", status ? "1" : "0");
  } else {
    server.send(400, "text/plain", "Invalid command");
  }
}

void handleDeleteCommand() {
  u_int16_t index = server.pathArg(0).toInt();
  if (index < memoryArraySize) {
    #ifdef DEBUG
    Serial.print("Deleting memory at index: ");
    Serial.println(index);
    #endif
    currentMemoryIndex = index;
    deleteMemory();
    server.send(200, "text/plain", "Memory deleted");
  } else {
    server.send(400, "text/plain", "Invalid memory index");
  }
}


void handleSaveCommand() {
  u_int32_t khz = server.pathArg(0).toInt();
  u_int16_t steps = server.pathArg(1).toInt();
  #ifdef DEBUG
  Serial.print("Received save command: ");
  Serial.print(khz);
  Serial.print(" ");
  Serial.println(steps);
  #endif
  addMemory(steps, khz);
  server.send(200, "text/plain", "OK");
  refreshTuningScreen();
}

void handleMemoryListCommand() {
  String json = "[";
  for (size_t i = 0; i < memoryArraySize; ++i) {
    json += "{\"index\":";
    json += String(i);
    json += ",\"steps\":";
    json += String(memoryArray[i].steps);
    json += ",\"khz\":";
    json += String(memoryArray[i].khz);
    json += ",\"selected\":";
    json += currentMemoryIndex == i ? "true" : "false";
    json += "}";
    if (i < memoryArraySize - 1) {
      json += ",";
    }
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleStatusCommand() {
  String json = "{\"currentSteps\":";
  json += String(stepper.currentPosition());
  json += ",\"currentFrequency\":";
  json += String(currentFrequency);
  json += ",\"currentMemory\":";
  json += String(currentMemoryIndex);
  json += ",\"capacity\":\"";
  json += String(calculatePF(stepper.currentPosition()), 2);
  json += "\",\"endstopSteps\":";
  json += String(stepper_endstop_steps);
  json += ",\"rigctldActive\":";
  json += rigctldActive ? "true" : "false";
  json += ",\"automaticMemorySelection\":";
  json += automaticMemorySelection ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}

void handleStepCommand() {
  String command = server.pathArg(0);
  int step = server.pathArg(1).toInt();
  #ifdef DEBUG
  Serial.print("Received step command: ");
  Serial.print(command);
  Serial.print(" ");
  Serial.println(step);
  #endif
  stepper_keep_enabled();
  if (command == "increase") {
    stepper_programmed_steps = constrain(stepper_programmed_steps + step, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    String json = "{\"status\":\"OK\",\"currentSteps\":";
    json += String(stepper_programmed_steps);
    json += "}";
    server.send(200, "application/json", json);
  } else if (command == "decrease") {
    stepper_programmed_steps = constrain(stepper_programmed_steps - step, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    String json = "{\"status\":\"OK\",\"currentSteps\":";
    json += String(stepper_programmed_steps);
    json += "}";
    server.send(200, "application/json", json);
  } else {
    server.send(400, "application/json", "{\"status\":\"ERROR\",\"description\":\"Invalid step command\"}");
  }
}

void handleMemoryAutoCommand() {
  bool command = server.pathArg(0) == "true";
  setAutoMemorySelection(command);
  String json = "{\"status\":\"OK\",\"automaticMemorySelection\":";
  json += automaticMemorySelection ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}

void handleRigctldControlCommand() {
  String command = server.pathArg(0);
  if (command == "true") {
    setRigctldActive(true);
  } else if (command == "false") {
    setRigctldActive(false);
  } else {
    server.send(400, "application/json", "{\"status\":\"ERROR\",\"description\":\"Invalid command\"}");
  }
  String json = "{\"status\":\"OK\",\"rigctldActive\":";
  json += rigctldActive ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}

void handleTuningCommand() {
  if (!rigctldActive) {
    server.send(400, "application/json", "{\"status\":\"ERROR\",\"description\":\"RIG control not active\"}");
    return;
  }
  bool measureOnly = server.pathArg(0) == "swr";
  float SWR = findMinimumSWRByRigctld(measureOnly);
  String json = "{\"status\":\"OK\",\"SWR\":";
  json += String(SWR, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleDisconnectWifi() {
  preferences.putBool("connect_to_wifi", false);
  server.send(200, "text/html", "<html><body><h1>WiFi Disconnected</h1><p>The controller will restart now.</p></body></html>");
  delay(2000);
  ESP.restart();
}

void handleSetupWifi() {
  if (server.method() == HTTP_POST) {
    String newSsid = server.arg("ssid");
    String newPassword = server.arg("password");
    String newRigctldHost = server.arg("rigctld_host");
    u_int16_t newRigctldPort = server.arg("rigctld_port").toInt();
    #ifdef DEBUG
    Serial.print("Received WiFi setup: ");
    Serial.print(newSsid);
    Serial.print(" ");
    Serial.print(newPassword);
    Serial.print(" ");
    Serial.print(newRigctldHost);
    Serial.print(" ");
    Serial.println(newRigctldPort);
    #endif

    if (newSsid.isEmpty() || newPassword.isEmpty()) {
      server.send(400, "text/plain", "SSID and Password cannot be empty");
      return;
    }

    preferences.putString(SSID_KEY, newSsid);
    preferences.putString(PASSWORD_KEY, newPassword);
    preferences.putString(RIGCTLD_HOST_KEY, newRigctldHost);
    preferences.putUInt(RIGCTLD_PORT_KEY, newRigctldPort);
    preferences.putBool("connect_to_wifi", true);

    String html = "<html><body><h1>WiFi Settings Saved</h1>";
    html += "<p>SSID: " + newSsid + "</p>";
    html += "<p>Password: " + newPassword + "</p>";
    html += "<p>Rigctld Host: " + newRigctldHost + "</p>";
    html += "<p>Rigctld Port: " + String(newRigctldPort) + "</p>";
    html += "<p>The controller will restart now.</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleUpdateendstop() {
  if (server.method() == HTTP_POST) {
    String newendstop = server.arg("endstop");
    if (newendstop.isEmpty() || !newendstop.toInt()) {
      server.send(400, "text/plain", "Invalid endstop value");
      return;
    }

    int endstopValue = newendstop.toInt();
    preferences.putInt(ENDSTOP_KEY, endstopValue); // Save as integer
    stepper_endstop_steps = endstopValue; // Update the stepper endstop steps

    #ifdef DEBUG
    Serial.print("Updated endstop to: ");
    Serial.println(endstopValue);
    #endif

    server.send(200, "text/plain", "Endstop updated successfully");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleGetEndstop() {
  int currentEndstop = preferences.getInt(ENDSTOP_KEY, 0); // Retrieve as integer
  server.send(200, "text/plain", String(currentEndstop));
}

void setupWebServer() {
    preferences.begin("wifi", false);
    String ssid = preferences.getString(SSID_KEY, "defaultSSID");
    String password = preferences.getString(PASSWORD_KEY, "defaultPassword");

    // Wokwi does not support Access Point mode.
    #ifdef EMULATOR
    bool connect_to_wifi = true;
    Serial.println("WARNING: EMULATOR mode is enabled. WiFi credentials are fixed.");
    showMessage("EMULATOR MODE", "DO NOT USE");
    delay(3000); // Display the warning for 3 seconds
    #else
    bool connect_to_wifi = preferences.getBool("connect_to_wifi", false);
    #endif

    if (connect_to_wifi) {
        #ifdef EMULATOR
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
        #else
        WiFi.begin(ssid.c_str(), password.c_str());
        #endif
        #ifdef DEBUG
        Serial.print("Connecting to WiFi ");
        Serial.println(ssid);
        Serial.print(" with password ");
        Serial.println(password);
        #endif
        lcd.setCursor(0, 0);
        lcd.println("CONNECTING WIFI ");
        lcd.setCursor(0, 1);
        lcd.println("PLEASE WAIT     ");
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
            delay(100);
            #ifdef DEBUG
            Serial.print(".");
            #endif
        }
        if (WiFi.status() != WL_CONNECTED) {
            #ifdef DEBUG
            Serial.println("WiFi Connection failed!");
            #endif
            preferences.putBool("connect_to_wifi", false); // Disable WiFi connection on failure
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        #ifdef DEBUG
        Serial.println("Starting in AP mode...");
        #endif
        WiFi.softAP("K1FM Loop Controller");
        IPAddress IP = WiFi.softAPIP();
        #ifdef DEBUG
        Serial.print("AP IP address: ");
        Serial.println(IP);
        #endif

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.println("AP MODE         ");
        lcd.setCursor(0, 1);
        lcd.print(IP);
        lcd.println("          ");

        server.on("/setup_wifi", handleSetupWifi); // Add handler for WiFi setup

    } else {
        #ifdef DEBUG
        Serial.println("WiFi Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        #endif

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.println("CONNECTED       ");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        lcd.println("          ");
        delay(2000);

    }

    setupAutomation(); // Ensure automation is initialized

    server.on(UriBraces("/select/{}"), []() {
      size_t index = server.pathArg(0).toInt();
      #ifdef DEBUG
      Serial.print("Selecting Memory #");
      Serial.println(index);
      #endif
      selectMemoryByIndex(index);
      printFrequency();
      sendHtml();
    });

    server.on(UriBraces("/adjust/{}"), handleAdjustCommand);
    server.on(UriBraces("/delete/{}"), handleDeleteCommand);
    server.on(UriBraces("/save/{}/{}"), handleSaveCommand);
    server.on(UriBraces("/tune/{}"), handleTuningCommand);
    server.on(UriBraces("/memories"), handleMemoryListCommand);
    server.on(UriBraces("/status"), handleStatusCommand);
    server.on(UriBraces("/step/{}/{}"), handleStepCommand);
    server.on(UriBraces("/memory_auto/{}"), handleMemoryAutoCommand);
    server.on(UriBraces("/rigctld_control/{}"), handleRigctldControlCommand);
    server.on("/disconnect_wifi", handleDisconnectWifi);
    server.on("/update_endstop", handleUpdateendstop);
    server.on("/get_endstop", handleGetEndstop); // Add handler for retrieving the endstop value
    server.on("/", sendHtml);

    server.begin();
    #ifdef DEBUG
    Serial.println("HTTP server started");
    #endif
    lcd.clear();
}

void remote_loop() {
  if (millis() - last_stepper_keep_enabled > 500 && 
      stepper.distanceToGo() == 0) {
    server.handleClient();
  }
}
