#ifndef STEPPER_H
#define STEPPER_H

#include <AccelStepper.h>
#include "debug.h"
#include "rotary.h"
#include "lcd.h"
#include "memories.h"
#include "remote.h"

#define STEPPER_DIR_PIN    27
#define STEPPER_STEP_PIN   25
#define STEPPER_ENABLE_PIN 12

// Programmed stepper position (steps)
extern u_int16_t stepper_programmed_steps;

// Stepper endstop position (steps)
extern u_int16_t stepper_endstop_steps;

// time since the stepper was last enabled
extern uint32_t last_stepper_keep_enabled;

extern AccelStepper stepper;

void stepper_loop();
void stepper_stop();
void stepper_keep_enabled();

#endif
