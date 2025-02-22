#include "remote.h"
#include "remote.html.h"

WebServer server(80);

void sendHtml() {
  server.send(200, "text/html", htmlContent);
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

void setupWebServer() {
  #ifdef EMULATOR
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  #else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #endif
  #ifdef DEBUG
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  #endif
  lcd.setCursor(0,0);
  lcd.println("CONNECTING WIFI ");
  lcd.setCursor(0,1);
  lcd.println("PLEASE WAIT     ");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(100);
    #ifdef DEBUG
    Serial.print(".");
    #endif
  }
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
    Serial.println("Wifi Connection timed out!");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Wifi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    #endif

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println("CONNECTED       ");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    lcd.println("          ");
    delay(2000);
  
    server.on("/", sendHtml);

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
    server.on(UriBraces("/memories"), handleMemoryListCommand);
    server.on(UriBraces("/status"), handleStatusCommand);
    server.on(UriBraces("/step/{}/{}"), handleStepCommand);
    server.on(UriBraces("/memory_auto/{}"), handleMemoryAutoCommand);
    server.on(UriBraces("/rigctld_control/{}"), handleRigctldControlCommand);

    server.begin();
    #ifdef DEBUG
    Serial.println("HTTP server started");
    #endif
  }

  lcd.clear();
  
}
void remote_loop() {
  if (millis() - last_stepper_keep_enabled > 500 && 
      stepper.distanceToGo() == 0 &&
      WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
}
