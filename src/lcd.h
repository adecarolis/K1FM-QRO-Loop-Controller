#ifndef LCD_H
#define LCD_H

#define SCL_PIN      26 // ZSTEP
#define SDA_PIN      17 // XSTEP

#include <LiquidCrystal_I2C.h>
#include "debug.h"
#include "stepper.h"
#include "memories.h"

extern LiquidCrystal_I2C lcd;

// LCD Functions
void printStepperPosition();
void printFrequency(u_int32_t frequency = 0);
void printCapacity();
void printSelectedSteps();
void printSelectedMemory(u_int16_t memoryIndex = UINT16_MAX);
void showMessage(String row1, String row2);
void refreshTuningScreen();
void printEndstop();
float calculatePF(int);

#endif