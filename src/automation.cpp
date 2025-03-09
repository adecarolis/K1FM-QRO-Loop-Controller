#include "automation.h"
#define MAXWAIT 5000

bool rigctldActive;

String rigctl_host;
uint16_t rigctl_port;

WiFiClient rigClient;

unsigned long lastUpdate = 0;

void setupAutomation() {
    #ifdef DEBUG
    Serial.println("Initializing automation...");
    #endif

    preferences.begin("wifi", true); // Ensure Preferences is initialized
    rigctl_host = preferences.getString(RIGCTLD_HOST_KEY, "");
    rigctl_port = preferences.getUInt(RIGCTLD_PORT_KEY, 4532);

    #ifdef DEBUG
    Serial.print("Loaded rigctl_host: ");
    Serial.println(rigctl_host);
    Serial.print("Loaded rigctl_port: ");
    Serial.println(rigctl_port);
    #endif
}

void automation_loop() {
    unsigned long currentTime = millis();
    // while rigctldActive is set we check
    // the current frequency every second
    if (currentTime - lastUpdate > 1000
        && rigctldActive
        && stepper.distanceToGo() == 0
        && !settingFrequency
        && !settingEndstop
        && !adjusting) {
        lastUpdate = currentTime;

        if (!rigClient.connected()) {
            connectToRigctld();
            if (!rigClient.connected()) {
                #ifdef DEBUG
                Serial.println("rigctld not connected");
                #endif
                return;
            }
        }

        u_int32_t frequency = getFrequencyByRigctld();

        if (currentFrequency != frequency) {

            // if the frequency changed, we update currentFrequency
            Serial.println("\nautomation_loop()");
            #ifdef DEBUG
            Serial.println("new frequency (rigctl):" + String(frequency));
            #endif
            currentFrequency = frequency;
            printFrequency();

            // if auomaticMemorySelection is also set
            // we call selectMemoryByFrequency
            if (automaticMemorySelection) {
                #ifdef DEBUG
                Serial.println("rigctld Frequency: " + String(currentFrequency));
                Serial.println("setting memory automatically");
                #endif
                selectMemoryByFrequency(currentFrequency);
            }
        }
    }
}

int rigctldConnectionAttempts = 0;

bool connectToRigctld() {
    #ifdef DEBUG
    Serial.println("\nconnectToRigctld()");
    #endif

    while (!rigClient.connected() && rigctldConnectionAttempts < 5) {
        if (!rigClient.connect(rigctl_host.c_str(), rigctl_port, 2000)) {
            rigctldConnectionAttempts++;
            #ifdef DEBUG
            Serial.print("Connection to rigctld failed! Attempt: ");
            Serial.println(rigctldConnectionAttempts);
            Serial.println("rigctl_host: " + rigctl_host);
            Serial.println("rigctl_port: " + String(rigctl_port));
            #endif
        } else {
            rigctldConnectionAttempts = 0;
            return true;
        }
    }

    if (rigctldConnectionAttempts >= 5) {
        #ifdef DEBUG
        Serial.println("Connection to rigctld failed 5 times, exiting...");
        #endif
        rigClient.stop();
        setRigctldActive(false);
        rigctldConnectionAttempts = 0;
    }
    return false;
}

void disconnectFromRigctld() {
    #ifdef DEBUG
    Serial.println("\ndisconnectFromRigctld()");
    #endif

    if (rigClient.connected()) {
        rigClient.stop();
    }
}


void sendCommand(const String& command) {
    rigClient.flush();
    rigClient.println(command);
    #ifdef DEBUG
    Serial.println("Command sent: " + command);
    #endif
}

String readResponse() {
    String response;
    unsigned long startTime = millis();
    while (millis() - startTime < MAXWAIT) {
        if (rigClient.available()) {
            response = rigClient.readStringUntil('\n');
            #ifdef DEBUG
            Serial.println("Response time: " + String(millis() - startTime));
            #endif
            break;
        }
        delay(100);  // Prevent CPU overload
    }
    #ifdef DEBUG
    Serial.println("Response: " + response);
    #endif
    return response;
}

bool setFrequencyByRigctld(uint32_t frequency) {
    
    #ifdef DEBUG
    Serial.println("\nsetFrequencyByRigctld()");    
    #endif

    String command = ",\\set_freq " + String(frequency) + "000";
    sendCommand(command);
    String response = readResponse();
    if (response.startsWith("set_freq: " + String(frequency) + "000")) {
      // Frequency was set correctly
      #ifdef DEBUG
      Serial.println("Frequency set to " + String(frequency) + " KHz");
      #endif
      return true;
    } else {
      // Error: unexpected response
      #ifdef DEBUG
      Serial.print("Unexpected response from rigctld");
      #endif
      return false;
    }
}

u_int32_t getFrequencyByRigctld() {
    #ifdef DEBUG
    Serial.println("\ngetFrequencyByRigctld()");
    #endif

    sendCommand(",\\get_freq");
    String response = readResponse();
    if (!response.startsWith("get_freq:,") || !response.endsWith("RPRT 0")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return 0;
    }

    int firstComma = response.indexOf(',');
    int secondComma = response.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return 0;
    }

    String freqRaw = response.substring(firstComma + 1, secondComma);
    if (!freqRaw.startsWith("Frequency: ")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return 0;
    }

    u_int32_t frequency = freqRaw.substring(10).toInt();
    return frequency / 1000;
}

float getSWRByRigctld() {
    #ifdef DEBUG
    Serial.println("\ngetSWRByRigctld()");
    #endif

    sendCommand(",\\get_level SWR");
    String response = readResponse();
    if (!response.startsWith("get_level: SWR,") || !response.endsWith("RPRT 0")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return -1;
    }

    int comma = response.indexOf(',', 13);
    if (comma == -1) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return -2;
    }

    float swr = response.substring(comma + 1, response.length() - 5).toFloat();
    return swr;
}

bool getModeByRigctld(String &mode, u_int16_t &bandwidth) {
    #ifdef DEBUG
    Serial.println("\ngetModeByRigctld()");
    #endif

    sendCommand(",\\get_mode");
    String response = readResponse();
    if (!response.startsWith("get_mode:,") || !response.endsWith("RPRT 0")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return false;
    }
    
    int firstComma = response.indexOf(',');
    int secondComma = response.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return false;
    }
    String modeRaw = response.substring(firstComma + 1, secondComma);
    if (!modeRaw.startsWith("Mode: ")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return false;
    }
    mode = modeRaw.substring(6);
    int thirdComma = response.indexOf(',', secondComma + 1);
    if (secondComma == -1 || thirdComma == -1) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return false;
    }
    String bandwidthRaw = response.substring(secondComma + 1, thirdComma);
    if (!bandwidthRaw.startsWith("Passband: ")) {
        #ifdef DEBUG
        Serial.println("Invalid response from rigctld, exiting...");
        #endif
        return false;
    }
    bandwidth = bandwidthRaw.substring(10).toInt();
    
    Serial.print("Mode: ");
    Serial.println(mode);
    Serial.print("Bandwidth: ");
    Serial.println(bandwidth);

    return true;
}

bool setModeByRigctld(String mode, u_int16_t bandwidth) {
    #ifdef DEBUG
    Serial.println("\nsetModeByRigctld()");
    #endif

    String command = ",\\set_mode " + mode + " " + String(bandwidth);
    sendCommand(command);
    String response = readResponse();
    return response.startsWith("set_mode: ") && response.endsWith("RPRT 0");
}

bool setPowerByRigctld(float power) {
    #ifdef DEBUG
    Serial.println("\nsetPowerByRigctld()");
    #endif

    String command = ",\\set_level RFPOWER " + String(power);
    sendCommand(command);
    String response = readResponse();
    return response.startsWith("set_level: RFPOWER ") && response.endsWith(",RPRT 0");
}

float getPowerByRigctld() {
    #ifdef DEBUG
    Serial.println("\ngetPowerByRigctld()");
    #endif

    sendCommand(",\\get_level RFPOWER");
    String response = readResponse();
    if (response.startsWith("get_level: RFPOWER,") && response.endsWith("RPRT 0")) {
        int comma = response.indexOf(',', 13);
        if (comma == -1) {
            #ifdef DEBUG
            Serial.println("Invalid response from rigctld, exiting...");
            #endif
            return -1;
        }
        float result = response.substring(comma + 1, response.length() - 5).toFloat();
        #ifdef DEBUG
        Serial.print("Power: ");
        Serial.println(result);
        #endif
        return result;
    }
    #ifdef DEBUG
    Serial.println("Invalid response from rigctld, exiting...");
    #endif
    return -2;
}

bool setTransmitByRigctld(bool transmit) {
    #ifdef DEBUG
    Serial.println("\nsetTransmitByRigctld()");
    #endif

    sendCommand(",\\set_ptt " + String(transmit ? 1 : 0));
    String answer = readResponse();
    return answer.startsWith("set_ptt: ") && answer.endsWith(",RPRT 0");
}

bool getTransmitByRigctld() {
    #ifdef DEBUG
    Serial.println("\ngetTransmitByRigctld()");
    #endif

    sendCommand(",\\get_level SWR");
    String response = readResponse();
    if (response.startsWith("get_level: SWR,") && response.endsWith("RPRT 0")) {
        int comma = response.indexOf(',', 13);
        if (comma == -1) {
            #ifdef DEBUG
            Serial.println("Invalid response from rigctld, exiting...");
            #endif
            return false;
        }
        float result = response.substring(comma + 1, response.length() - 5).toFloat();
        #ifdef DEBUG
        Serial.print("SWR: ");
        Serial.println(result);
        #endif
        return result > 1.0;
    }
    #ifdef DEBUG
    Serial.println("Invalid response from rigctld, exiting...");
    #endif
    return false;
}

float transmitAndReturnSWRByRigctld() {

    #ifdef DEBUG
    Serial.println("\ntransmitAndReturnSWRByRigctld()");
    #endif

    float SWR = -1;

    #ifdef EMULATOR
    #ifdef DEBUG
    Serial.println("Emulator mode enabled");
    #endif
    
    SWR = 2.7;
    static int16_t startSteps = 0;
    int deltaSteps = stepper.currentPosition() - startSteps;
    if (startSteps == 0) {
        startSteps = stepper.currentPosition();
        return SWR;
    }
    float swrValues[] = { 5.0, 3.5, 3.0, 2.2, 1.7, 1.3, 1.1, 1.3, 2.9, 3.0, 3.4, 3.9, 5.0 };
    int swrValuesCount = sizeof(swrValues) / sizeof(swrValues)[0];
    int swrIndex = (abs(deltaSteps) - 1) / 5 + random(0, swrValuesCount - 1);
    if (swrIndex >= swrValuesCount) {
        swrIndex = swrValuesCount - 1;
    }
    SWR = swrValues[swrIndex];
    return SWR;
    #endif

    if (setTransmitByRigctld(true)) {
      #ifdef DEBUG
      Serial.println("Transmit SET");
      #endif
    }

    int attempts = 0;
    while (attempts < 3 && SWR < 1.0) {
      SWR = getSWRByRigctld();
      attempts++;
    }
    if (attempts >= 3) {
      SWR = -1;
    }

    int retryCount = 0;
    while (retryCount < 10 && !setTransmitByRigctld(false)) {
      #ifdef DEBUG
      Serial.println("Retrying to set transmit OFF");
      #endif
      delay(1000);
      retryCount++;
    }
    if (retryCount < 10) {
      #ifdef DEBUG
      Serial.println("Transmit OFF");
      #endif
    } else {
      #ifdef DEBUG
      Serial.println("Could not verify transmit is OFF");
      #endif
      return 0;
    }

    return SWR;

}

float findMinimumSWRByRigctld(bool measureOnly) {
  int stepsToMove = 20;
  int step = 5;
  u_int16_t startPosition = stepper.currentPosition();
  int bestStep = startPosition;

  uint16_t bw;
  String mode;

  // Set the radio up for FM transmission
  if (getModeByRigctld(mode, bw)) {
    #ifdef DEBUG
    Serial.println("Saving current mode");
    #endif
  }

  if (setModeByRigctld("FM", 3000)) {
    #ifdef DEBUG
    Serial.println("FM SET");
    #endif
  }

  float power = getPowerByRigctld();

  if (power >= 0) {
    #ifdef DEBUG
    Serial.print("Power: ");
    Serial.println(power);
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("No Power");
    #endif
    return 0;
  }

  if (setPowerByRigctld(0.1)) {
    #ifdef DEBUG
    Serial.println("Power SET");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Power not set");
    #endif
    return 0;
  }

  float minSWR = transmitAndReturnSWRByRigctld();
  #ifdef DEBUG
  Serial.println("\nfindMinimumSWRByRigctld()");
  Serial.print("Initial SWR: ");
  Serial.println(minSWR);
  Serial.print("Initial Position: ");
  Serial.println(stepper.currentPosition());
  #endif

  float currentSWR = minSWR;

  int direction = 1;
  int attempt = 0;

  while (minSWR > 1.2 && attempt < 10 && measureOnly == false) {

    stepper_keep_enabled();
    stepper.move(direction * step);
    stepper.setSpeed(100);
    stepper.runToPosition();
    currentSWR = transmitAndReturnSWRByRigctld();

    // if we moved in the positive direction and the SWR is worse than 
    // the minimum so far then we need to move in the negative direction
    // otherwise we move in the positive direction
    if (attempt == 0) {
        if (currentSWR > minSWR) {
            direction = -direction;
        } 
        minSWR = currentSWR;
        bestStep = stepper.currentPosition();
    } else if (attempt > 0 and step == 5) {
        if (currentSWR < minSWR) {
            minSWR = currentSWR;
            bestStep = stepper.currentPosition();
        } else {
            direction = -direction;
            step = 1;
        }
    } else if (attempt > 0 and step == 1) {
        if (currentSWR < minSWR) {
            minSWR = currentSWR;
            bestStep = stepper.currentPosition();
        } else {
            stepper_keep_enabled();
            stepper.moveTo(bestStep);
            stepper.setSpeed(100);
            stepper.runToPosition();
            r.resetPosition(bestStep);
            break;
        }
    }
    
    #ifdef DEBUG
    Serial.print("Attempt: "); 
    Serial.print(attempt);
    Serial.print(" Steps: ");
    Serial.print(stepper.currentPosition());
    Serial.print(" SWR: ");
    Serial.println(currentSWR);
    #endif

    attempt++;
    delay(50);
  }
  #ifdef DEBUG
  Serial.print("\nSolution: ");
  Serial.print(stepper.currentPosition());
  Serial.print(" Attempt: ");
  Serial.print(attempt);
  Serial.print(" SWR: ");
  Serial.println(minSWR);
  #endif

  // Restore the radio settings
  if (setModeByRigctld(mode, bw)) {
    #ifdef DEBUG
    Serial.println("Mode restored");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Mode not restored");
    #endif
    return 0;
  }

  if (setPowerByRigctld(power)) {
    #ifdef DEBUG
    Serial.println("Power restored");
    #endif
  } else {
    #ifdef DEBUG  
    Serial.println("Power not restored");
    #endif
    return 0;
  }

  return minSWR;
}