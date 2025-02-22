#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <WiFiClient.h>
#include "memories.h"

u_int32_t getFrequencyByRigctld();
bool setFrequencyByRigctld(uint32_t frequency);

extern bool rigctldActive;
void automation_loop();

#endif