#ifndef BUTTONS_H
#define BUTTONS_H

#include <Button.h>
#include "stepper.h"
#include "rotary.h"
#include "memories.h"
#include "lcd.h"
#include "debug.h"
#include "menu.h"

#define BUTTON1_PIN  13 // ENDSTOP X-
#define BUTTON2_PIN  5  // ENDSTOP Y-
#define BUTTON3_PIN  23 // ENDSTOP Z- 
#define BUTTON4_PIN  14 // ZDIR
#define BUTTON5_PIN  18 // SPINDIR

extern Button button1;
extern Button button2;
extern Button button3;
extern Button button4;
extern Button button5;

u_int16_t adjustingUnset(); 
void button_loop();

#endif
