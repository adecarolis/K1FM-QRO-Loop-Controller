#include "lcd.h"

LiquidCrystal_I2C lcd(0x27,16,2);

float calculatePF(int encoderValue) {
    if (encoderValue < 0 || encoderValue > 22200) {
        // Out of bounds, return error value or handle as needed
        return -1;
    }

    // Range 1: 0 to 18000 steps
    if (encoderValue <= 18000) {
        float m = (25.9f - 500.0f) / (18000.0f - 0.0f); // Slope
        float b = 500.0f; // Intercept
        return m * encoderValue + b;
    }

    // Range 2: 18000 to 20500 steps
    else if (encoderValue <= 20500) {
        float m = (7.0f - 25.9f) / (20500.0f - 18000.0f); // Slope
        float b = 25.9f - m * 18000.0f; // Intercept
        return m * encoderValue + b;
    }

    // Range 3: 20500 to 22200 steps
    else {
        float m = (5.0f - 7.0f) / (22200.0f - 20500.0f); // Slope
        float b = 7.0f - m * 20500.0f; // Intercept
        return m * encoderValue + b;
    }
}

void showMessage(String row1, String row2) {
  lcd.clear();
  lcd.print(row1);
  for (int i = 0; i < 16 - row1.length(); ++i) {
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);
  lcd.print(row2);
  for (int i = 0; i < 16 - row2.length(); ++i) {
    lcd.print(" ");
  }
  delay(2000);
  lcd.clear();
}

void printEndstop() {
  lcd.setCursor(0, 0);
  lcd.print("ENDSTOP: ");
  lcd.setCursor(0, 1);
  lcd.print(stepper_endstop_steps);
}

void printCapacity() {
  lcd.setCursor(6,0);
  lcd.print("       ");
  lcd.setCursor(6,0);
  lcd.print("  ");
  lcd.print(calculatePF(stepper.currentPosition()));
  lcd.print("pF  ");
}

void printZeroPadded(int number) {
  if (number < 10) lcd.print("0000");
  else if (number < 100) lcd.print(F("000"));
  else if (number < 1000) lcd.print(F("00"));
  else if (number < 10000) lcd.print(F("0"));
  lcd.print(number);
}

void printSelectedSteps() {
  lcd.setCursor(5,0);
  lcd.print("       ");
  lcd.setCursor(9,0);
  lcd.print("->");
  lcd.print(stepper_programmed_steps);
  lcd.print("       ");
}

void printStepperPosition() {

    lcd.setCursor(0, 0);

    if (adjusting) {
      lcd.print(adjustPosition);
      lcd.print("  ");
      lcd.print(stepper.currentPosition() - adjustPosition);
      lcd.print("  ");
    } else {
      lcd.print(stepper.currentPosition());
      lcd.print("   ");
      if (stepper.currentPosition() != stepper_programmed_steps) {
        printSelectedSteps();
      }
    }
}

void printFrequency(u_int32_t frequency) {

  lcd.setCursor(0, 1);
  printZeroPadded(frequency ? frequency : currentFrequency);
  lcd.print(F(" KHz"));

}

void printSelectedMemory(u_int16_t memoryIndex) {

  if (memoryIndex == UINT16_MAX) memoryIndex = currentMemoryIndex;

  if (memoryIndex >= memoryArraySize) {
    return; // Out of bounds, do nothing
  }

  StepData memory = memoryArray[memoryIndex];

  if (currentFrequency == memory.khz && stepper_programmed_steps == memory.steps) {
    lcd.setCursor(13, 1);
    lcd.print("M");
    lcd.print(memoryIndex);
    lcd.print(" ");
  } else {
    lcd.setCursor(13, 1);
    lcd.print("   ");
  }

}

void refreshTuningScreen() {
  printStepperPosition();
  printFrequency();
}
