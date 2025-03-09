#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <WiFiClient.h>
#include "memories.h"
#include <Preferences.h>

u_int32_t getFrequencyByRigctld();
bool setFrequencyByRigctld(uint32_t frequency);

extern bool rigctldActive;
extern Preferences preferences;

bool connectToRigctld();
void disconnectFromRigctld();
float getSWRByRigctld();
bool setModeByRigctld(String mode, u_int16_t bandwidth);
bool getModeByRigctld(String &mode, u_int16_t &bandwidth);
bool setTransmitByRigctld(bool transmit);
bool getTransmitByRigctld();
bool setPowerByRigctld(float power);
float getPowerByRigctld();
void automation_loop();
float transmitAndReturnSWRByRigctld();
float findMinimumSWRByRigctld(bool measureOnly = false);
void setupAutomation();

#endif