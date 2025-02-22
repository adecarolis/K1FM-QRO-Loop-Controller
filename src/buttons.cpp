#include "buttons.h"
Button button1(BUTTON1_PIN), button2(BUTTON2_PIN), button3(BUTTON3_PIN), button4(BUTTON4_PIN), button5(BUTTON5_PIN);


static uint32_t upButtonPressedTime = 0;

u_int16_t adjustingUnset() {
  adjusting = false;
  stepper.setCurrentPosition(adjustPosition);
  r.resetPosition(adjustPosition);
  refreshTuningScreen();
  return adjustPosition;
}

u_int16_t unsetEndstopAdjustment() {
  settingEndstop = false;
  r.setUpperBound(stepper_endstop_steps);
  r.resetPosition(stepper.currentPosition());
  refreshTuningScreen();
  #ifdef DEBUG
  Serial.println("\nunsetEndstopAdjustment()");
  Serial.println("Saving ENDSTOP:" + String(stepper_endstop_steps));
  #endif
  preferences.putInt(ENDSTOP_KEY, stepper_endstop_steps);
  return settingEndstop;
}

void adjusting_button_loop() {
  if (button3.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\n\nAdjusting UP Button Pressed");
    #endif
    stepper_keep_enabled();
    stepper_programmed_steps = constrain(stepper_programmed_steps + 1, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    printStepperPosition();
  } else if (button5.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\n\nAdjusting DOWN Button Pressed");
    #endif
    stepper_keep_enabled();
    stepper_programmed_steps = constrain(stepper_programmed_steps - 1, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    printStepperPosition();
  } else if (button4.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nAdjusting Menu Button Pressed");
    #endif
    adjustingUnset();
  }
}

void endstop_button_loop() {
  if (button3.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\n\nEndstop UP Button Pressed");
    #endif
    stepper_endstop_steps = constrain(stepper_endstop_steps + 10, 0, UINT16_MAX);
    r.resetPosition(stepper_endstop_steps);
    printEndstop();
    delay(200);
  } else if (button5.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\n\nEndstop DOWN Button Pressed");
    #endif
    stepper_endstop_steps = constrain(stepper_endstop_steps - 10, 0, UINT16_MAX);
    r.resetPosition(stepper_endstop_steps);
    printEndstop();
    delay(200);
  } else if (button2.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\nEndstop Memory DOWN Button Pressed");
    #endif
    stepper_endstop_steps -= 1000;
    r.resetPosition(stepper_endstop_steps);
    printEndstop();
    delay(500);
  } else if (button1.read() == Button::PRESSED) {
    #ifdef DEBUG
    Serial.println("\nEndstop Memory UP Button Pressed");
    #endif
    stepper_endstop_steps += 1000;
    r.resetPosition(stepper_endstop_steps);
    printEndstop();
    delay(500);
  } else if (button4.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nEndstop Menu Button Pressed");
    #endif
    unsetEndstopAdjustment();
  }
}

void frequency_button_loop() {
  if (button3.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nFrequency UP Button Pressed");
    #endif
    currentFrequency += 1000;
    r.resetPosition(currentFrequency);
    printFrequency(currentFrequency);
  } else if (button5.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nFrequency DOWN Button Pressed");
    #endif
    currentFrequency -= 1000;
    r.resetPosition(currentFrequency);
    printFrequency(currentFrequency);
  } else if (button1.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nnFrequency Memory UP Button Pressed");
    #endif
    currentFrequency += 100;
    r.resetPosition(currentFrequency);
    printFrequency(currentFrequency);
  } else if (button2.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nFrequency Memory DOWN Button Pressed");
    #endif
    currentFrequency -= 100;
    r.resetPosition(currentFrequency);
    printFrequency(currentFrequency);
  } else if (button4.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nFrequency Menu Button Pressed");
    #endif
    r.setUpperBound(stepper_endstop_steps);
    settingFrequency = false;
    addMemory(stepper_programmed_steps, currentFrequency);
    refreshTuningScreen();
  }
}

void button_loop() {

  // if we are setting the frequency then configure
  // the buttons for frequency setting
  if (settingFrequency) {
    frequency_button_loop();
    return;
  }

  // if we are setting the endstop then configure
  // the buttons for endstop setting
  if (settingEndstop) {
    endstop_button_loop();
    return;
  }

  // if the adjusting function is active then configure
  // the buttons for adjusting
  if (adjusting) {
    adjusting_button_loop();
    return;
  }

  // If the stepper is moving only check
  // the menu button
  if (button4.pressed()) {
    if (stepper.distanceToGo() != 0) {
        #ifdef DEBUG
        Serial.println("\nMenu Button Pressed");
        #endif
        stepper_stop();
    } else {
      Serial.println("\nShowing Menu");
      menuVisible = true;
      menu.reset();
      menu.setCursor(0);
      menu.show();
    }
    return;
  }

  // otherwise use the buttons normally
  if (button1.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nMemory UP Button Pressed");
    #endif
    memoryUp();   
  } else if (button2.pressed()) {
    #ifdef DEBUG
    Serial.println("\n\nMemory DOWN Button Pressed");
    #endif
    memoryDown();
  } else if (button3.read() == Button::PRESSED) {
    stepper_keep_enabled();
    #ifdef DEBUG
    Serial.println("\nstepper_endstop_steps:" + String(stepper_endstop_steps));
    #endif
    stepper_programmed_steps = constrain(stepper_programmed_steps + 50, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    delay(100);
    #ifdef DEBUG
    Serial.println("\n\nUP Button Pressed");
    #endif
    printStepperPosition();
  } else if (button5.read() == Button::PRESSED) {
    stepper_keep_enabled();
    stepper_programmed_steps = constrain(stepper_programmed_steps - 50, 0, stepper_endstop_steps);
    stepper.moveTo(stepper_programmed_steps);
    delay(100);
    #ifdef DEBUG
    Serial.println("\n\nDOWN Button Pressed");
    #endif
    printStepperPosition();
  }
}

