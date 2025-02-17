#ifndef MEMORIES_H
#define MEMORIES_H

#include <Preferences.h>
#include "rotary.h"
#include "stepper.h"
#include "lcd.h"
#include "debug.h"

#define NAMESPACE "loop-memory"
#define MEMORIES_KEY "memories"
#define CURRENT_MEMORY_INDEX_KEY "memory_index"
#define CURRENT_MEMORY_SIZE_KEY "memory_array"
#define STEPS_KEY "steps"
#define KHZ_KEY "khz"
#define ENDSTOP_KEY "endstop"
#define MEMORY_MAX_SIZE 50

struct StepData {
    long steps;
    u_int32_t khz;
};

extern StepData memoryArray[];
extern Preferences preferences;
extern u_int16_t currentMemoryIndex;
extern u_int32_t currentFrequency;
extern u_int16_t memoryArraySize;
extern u_int16_t previewMemoryIndex;
extern bool adjusting;
extern u_int16_t adjustPosition;

void storeLong(const char* key, long value);

int retrieveInt(const char* key, int defaultValue);

long retrieveLong(const char* key, long defaultValue);

void storeStructArray(const char* key, StepData* array, size_t length);

void retrieveStructArray(const char* key, StepData* array, size_t length);

void addMemory(long steps, u_int32_t khz);

void deleteMemory();

void deleteAllMemories();

void memoryUp();

void memoryDown();

void updateCurrentMemoryIndex(u_int16_t index);

void selectMemoryByIndex(u_int16_t index);

void debugPrintMemoryArray();

void storeStructArray(const char* key, StepData* array, size_t length);

void updatePosition();

void storeMemories();

void memory_loop();

#endif