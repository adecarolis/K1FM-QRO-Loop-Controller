#ifndef ROTARY_H
#define ROTARY_H

#include "ESPRotary.h"
#include "stepper.h"
#include "lcd.h"
#include "debug.h"
#include "menu.h"

// Rotary Encoder Definitions
#define ROTARY_PIN1     19  // SPINENABLE
#define ROTARY_PIN2     16  // XDIR

#define CLICKS_PER_STEP 4
//#define MIN_POS         0
//#define MAX_POS         10000
#define INCREMENT       1
#define SPEEDUP_STEPS   20

extern ESPRotary r;

// Rotary Encoder Functions
void speedupStarted(ESPRotary& r);
void speedupEnded(ESPRotary& r);
void rotate(ESPRotary &r);
void upper(ESPRotary& r);
void lower(ESPRotary& r);
void showDirection(ESPRotary& r);

#endif