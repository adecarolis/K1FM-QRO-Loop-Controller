#include "rotary.h"

ESPRotary r;

void rotate(ESPRotary& r) {
   uint16_t position = r.getPosition();
   
   #ifdef DEBUG
   Serial.println("\nrotate()");
   #endif
   
   if (settingFrequency) {
      currentFrequency = position;
      #ifdef DEBUG
      Serial.println("New Frequency: " + String(currentFrequency));
      #endif
      printFrequency(currentFrequency);
   } else if (settingEndstop) {
      stepper_endstop_steps = position;
      #ifdef DEBUG
      Serial.println("New Endstop: " + String(currentFrequency));
      #endif
      printEndstop();
   } else {
      stepper_keep_enabled();
      stepper.runToNewPosition(position);
      stepper_programmed_steps = stepper.currentPosition();
      #ifdef DEBUG
      Serial.println("Current Encoder Position: " + String(position));
      Serial.println("New Encoder Position: " + String(stepper_programmed_steps));
      #endif
      printStepperPosition();
   }
}
