#include "rotary.h"
#include "buttons.h"
#include "lcd.h"
#include "stepper.h"
#include "memories.h"
#include "menu.h"
#include "remote.h"
#include "debug.h"
#include <Preferences.h>

// Define the preferences object here
Preferences preferences;

#define SERIAL_SPEED 115200

void setup() {

   pinMode(STEPPER_ENABLE_PIN, OUTPUT);

   // Serial Monitor Init
   Serial.begin(SERIAL_SPEED);
   delay(50);

   // Wire init
   // SCL/SDA pins are unreachable when using the
   // Protoneer CNC HAT on a Wemos D1 R32 board, therefore
   // we need to set custom pins
   Wire.begin(SCL_PIN, SDA_PIN);

   // Buttons Init
   button1.begin(); // Memory Up
   button2.begin(); // Memory Down
   button3.begin(); // Up
   button4.begin(); // Menu
   button5.begin(); // Down

   // Menu and LCD Init
   menu.setScreen(mainScreen);
   menu.hide();
   renderer.begin();

   // Memories Init
   preferences.begin(NAMESPACE, false);
   long initialPosition = retrieveLong(STEPS_KEY, 0);
   
   retrieveStructArray(MEMORIES_KEY, memoryArray, MEMORY_MAX_SIZE);
   memoryArraySize = retrieveInt(CURRENT_MEMORY_SIZE_KEY, 0);
   currentMemoryIndex = retrieveInt(CURRENT_MEMORY_INDEX_KEY, 0);
   previewMemoryIndex = currentMemoryIndex;
   currentFrequency = retrieveInt(KHZ_KEY, 0);
   automaticMemorySelection = retrieveBool(AUTOMATIC_MEMORY_SELECTION_KEY, false);
   rigctldActive = retrieveBool(RIGCTLD_ACTIVE_KEY, false);

   #ifdef DEBUG
   Serial.print("Initial stepper position: ");
   Serial.println(initialPosition);
   Serial.print("Memory Array Size: ");
   Serial.println(memoryArraySize);
   Serial.print("Current Memory Index: ");
   Serial.println(currentMemoryIndex);
   Serial.print("Current Frequency: ");
   Serial.println(currentFrequency);
   debugPrintMemoryArray();
   #endif

   // Defined in debug.h.
   // Use this once only if you need to load some default
   // memories. Don't forget to comment LOAD_MEMORIES
   // after you're done.
   #ifdef LOAD_MEMORIES
   addMemory(0, 4026);
   addMemory(14247, 10100);
   addMemory(15783, 14074);
   addMemory(16437, 18100);
   addMemory(16757, 21000);
   addMemory(16762, 21074);
   addMemory(18345, 28074);
   addMemory(18435, 28500);
   addMemory(18746, 29000);
   #endif

   // The Wokwi emulator does not support persistent storage.
   // Setting some values to emulate it.
   #ifdef EMULATOR
   currentMemoryIndex = 2;
   previewMemoryIndex = currentMemoryIndex;
   currentFrequency = 5335;
   initialPosition = 6657;
   stepper_endstop_steps = 22000;

   addMemory(0, 4026);
   addMemory(11039, 7074);
   addMemory(15783, 14074);
   addMemory(16437, 18100);
   addMemory(16762, 21074);
   addMemory(18345, 28074);
   #endif

   // Stepper Init
   stepper.setCurrentPosition(initialPosition);
   #ifndef EMULATOR
   stepper_endstop_steps = retrieveInt(ENDSTOP_KEY, 0);
   setRigctldActive(false);
   #endif
   stepper.setMaxSpeed(2000.0);
   stepper.setAcceleration(500.0);
   stepper.setSpeed(2000.0);

   // This is an anomaly where the current position is
   // higher than the endstop.
   // If it happens (it shouldn't) we set the endstop
   // to the current position + 1 so that we can move the
   // stepper and fix the problem.
   if (initialPosition > stepper_endstop_steps) {
      showMessage("ENDSTOP ERROR",
         String(stepper_endstop_steps) +
         " < " + String(initialPosition));
      #ifdef DEBUG
      Serial.println("\nEndstop ERROR:");
      Serial.print("Initial position: ");
      Serial.println(initialPosition);
      Serial.print("stepper_endstop_steps: ");
      Serial.println(stepper_endstop_steps);
      #endif
      stepper_endstop_steps = initialPosition + 1;
   }

   // Rotary Init
   r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, 
      0, stepper_endstop_steps, initialPosition, INCREMENT);

   r.setChangedHandler(rotate);
   r.retriggerEvent(false);
   r.enableSpeedup(false);
   r.resetPosition(initialPosition);

   // Webserver Init
   setupWebServer();

   printFrequency(currentFrequency);
   printSelectedMemory(currentMemoryIndex);
}

void loop() {

   if (menuVisible) {
      menu_loop();
   } else {
      button_loop();
      stepper_loop(); // calls rotary_loop when/if necessary
      memory_loop();
      remote_loop();
      automation_loop();
   }
}