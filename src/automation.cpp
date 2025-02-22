#include "automation.h"

bool rigctldActive;

// Replace with your rigctld server IP
const char* rigctl_host = "192.168.67.103";
const uint16_t rigctl_port = 4533;

WiFiClient rigClient;

unsigned long lastUpdate = 0;

void automation_loop() {
    unsigned long currentTime = millis();
    // while rigctldActive is set we check
    // the current frequency every second
    if (currentTime - lastUpdate > 1000
        && rigctldActive
        && stepper.distanceToGo() == 0
        && !settingFrequency
        && !settingEndstop) {
        lastUpdate = currentTime;
        u_int32_t frequency = getFrequencyByRigctld();

        // if it returns 0, rigctld isn't working
        if (frequency == 0) {
            // after 10 consecutive failures,
            // we set rigctldActive to false
            static int zeroCounter = 0;
            if (frequency == 0) {
                zeroCounter++;
                if (zeroCounter >= 10) {
                    #ifdef DEBUG
                    Serial.println("\nautomation_loop()");
                    Serial.println("rigctld is not working");
                    Serial.println("setting rigctldActive=false");
                    #endif
                    setRigctldActive(false);
                    zeroCounter = 0;
                }
            } else {
                zeroCounter = 0;
            }
            return;
        }

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

bool setFrequencyByRigctld(u_int32_t frequency) {

    #ifdef DEBUG
    Serial.println("\nsetFrequencyByRigctld()");
    #endif
    
    if (rigClient.connect(rigctl_host, rigctl_port)) {
        rigClient.println("F " + String(frequency * 1000));

        unsigned long startTime = millis();  // Start a timeout timer
        while (millis() - startTime < 3000) {  // Wait up to 3 seconds
            if (rigClient.available()) {
                String ret = rigClient.readStringUntil('\n');
                #ifdef DEBUG
                Serial.print("rigctld returns: ");
                Serial.println(ret);
                #endif
                return true;
                break;  // Exit loop after receiving data
            }
            delay(100);  // Prevent CPU overload
        }

        if (!rigClient.available()) {
            #ifdef DEBUG
            Serial.println("No response from rigctld, exiting...");
            #endif
        }
    } else {
        
        #ifdef DEBUG
        Serial.println("Connection to rigctld failed!");
        #endif
    }

    return false;

}

u_int32_t getFrequencyByRigctld() {
    
    if (rigClient.connect(rigctl_host, rigctl_port)) {
        rigClient.println("f");

        unsigned long startTime = millis();  // Start a timeout timer
        while (millis() - startTime < 3000) {  // Wait up to 3 seconds
            if (rigClient.available()) {
                String freq = rigClient.readStringUntil('\n');
                //Serial.print("Frequency: ");
                //Serial.println(freq);
                return freq.toInt() / 1000;
                break;  // Exit loop after receiving data
            }
            delay(100);  // Prevent CPU overload
        }

        if (!rigClient.available()) {
            #ifdef DEBUG
            Serial.println("No response from rigctld, exiting...");
            #endif
        }
    } else {
        #ifdef DEBUG
        Serial.println("Connection to rigctld failed!");
        #endif
    }

    return 0;
}
