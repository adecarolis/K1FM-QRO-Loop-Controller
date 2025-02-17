#include "stepper.h"

uint32_t last_stepper_keep_enabled = 0;
u_int16_t stepper_current_steps;
u_int16_t stepper_programmed_steps;
u_int16_t stepper_endstop_steps;
AccelStepper stepper(AccelStepper::DRIVER, STEPPER_STEP_PIN, STEPPER_DIR_PIN);

void stepper_loop() {

    if (millis() - last_stepper_keep_enabled > 500 &&
        digitalRead(STEPPER_ENABLE_PIN) == LOW &&
        stepper.distanceToGo() == 0) {

        digitalWrite(STEPPER_ENABLE_PIN, HIGH);
        if (stepper.currentPosition() > 0 && stepper.currentPosition() < stepper_endstop_steps) {
            stepper_programmed_steps = stepper.currentPosition();
            r.resetPosition(stepper_programmed_steps);
            #ifdef DEBUG
            Serial.println("\nstepper_loop()");
            Serial.println("Disabled Stepper");
            Serial.println("Stepper Programmed Position: " + String(stepper_programmed_steps));
            #endif
        }

        printStepperPosition();
        if (!adjusting) {
            printCapacity();
            printSelectedMemory();
            updatePosition();
        }
    }

    if (stepper.distanceToGo() != 0) {

       stepper.run();

    } else {
        // only poll the rotary encoder when the stepper is not moving
        r.loop();
    }

}

void stepper_stop() {

    #ifdef DEBUG
    Serial.println("Stop pressed at " + String(stepper.currentPosition()));
    #endif
    stepper.setAcceleration(4000);
    stepper.stop();
    while (stepper.distanceToGo() != 0) {
        stepper.run();
    }
    stepper_programmed_steps = stepper.currentPosition();
    Serial.println("Stopped at " + String(stepper_programmed_steps));
    r.resetPosition(stepper_programmed_steps);
    stepper.setAcceleration(500);
    lcd.clear();
    lcd.print("STOP! ");
    delay(2000);

}

void stepper_keep_enabled() {
  digitalWrite(STEPPER_ENABLE_PIN, LOW);
  last_stepper_keep_enabled = millis();
}
