#include "memories.h"

Preferences preferences;
StepData memoryArray[MEMORY_MAX_SIZE];
u_int16_t previewMemoryIndex = 0;
u_int16_t currentMemoryIndex = 0;
u_int32_t currentFrequency = 0;
u_int16_t memoryArraySize = 0;
bool adjusting = false;
bool automaticMemorySelection = false;
u_int16_t adjustPosition = 0;

void debugPrintMemoryArray() {
    // Create a temporary array to store the saved memory array
    StepData tempMemoryArray[MEMORY_MAX_SIZE];

    // Load the saved memory array into the temporary array
    retrieveStructArray(MEMORIES_KEY, tempMemoryArray, MEMORY_MAX_SIZE);

    // Print the temporary array
    Serial.println("\nMemory Array Size: " + String(memoryArraySize));
    Serial.println("Saved Memory Array Size: " + String(retrieveInt(CURRENT_MEMORY_SIZE_KEY, 0)));
    Serial.println("Saved Current Memory Index: " + String(retrieveInt(CURRENT_MEMORY_INDEX_KEY, 0)));  
    Serial.println("Saved Current Frequency: " + String(retrieveInt(KHZ_KEY, 0)));
    Serial.println("Saved Memory Array Contents:");
    for (size_t i = 0; i < memoryArraySize; ++i) {
        if (tempMemoryArray[i].steps != 0 || tempMemoryArray[i].khz != 0) {
            Serial.print("Index ");
            Serial.print(i);
            Serial.print(": kHz = ");
            Serial.print(tempMemoryArray[i].khz);
            Serial.print(", Steps = ");
            Serial.println(tempMemoryArray[i].steps);
        }
    }
}

void storeLong(const char* key, long value) {
    preferences.putLong(key, value);
}

int lastSavedValue = -1;

void updatePosition() {
    long value = r.getPosition();
    if (value != lastSavedValue) {
        #ifdef DEBUG
            Serial.print("Saving stepper position: ");
            Serial.println(value);
        #endif
        storeLong(STEPS_KEY, value);
        lastSavedValue = value;
    }
}

void storeMemories() {
    storeStructArray(MEMORIES_KEY, memoryArray, MEMORY_MAX_SIZE);
    preferences.putInt(CURRENT_MEMORY_INDEX_KEY, currentMemoryIndex);
    preferences.putInt(KHZ_KEY, currentFrequency);
    preferences.putInt(CURRENT_MEMORY_SIZE_KEY, memoryArraySize);
    #ifdef DEBUG
    debugPrintMemoryArray();
    #endif
}

bool retrieveBool(const char* key, bool defaultValue) {
    return preferences.getBool(key, defaultValue);
}

int retrieveInt(const char* key, int defaultValue) {
    return preferences.getInt(key, defaultValue);
}

long retrieveLong(const char* key, long defaultValue) {
    return preferences.getLong(key, defaultValue);
}

void storeStructArray(const char* key, StepData* array, size_t length) {
    preferences.putBytes(key, array, sizeof(StepData) * length);
}

void retrieveStructArray(const char* key, StepData* array, size_t length) {
    preferences.getBytes(key, array, sizeof(StepData) * length);
}

void storeCurrentMemoryIndex(u_int16_t index) {
    currentMemoryIndex = index;
    #ifdef DEBUG
    Serial.print("Current Memory Index: ");
    Serial.println(currentMemoryIndex);
    #endif
    preferences.putInt(CURRENT_MEMORY_INDEX_KEY, currentMemoryIndex);
}

void selectMemoryByFrequency(u_int32_t khz) {
    if (memoryArraySize == 0) {
        return;
    }

    #ifdef DEBUG
    Serial.println("\nselectMemoryByFrequency()");
    #endif

    // Find the memory with the exact frequency
    for (size_t i = 0; i < memoryArraySize; ++i) {
        if (memoryArray[i].khz == khz) {
            if (memoryArray[i].steps > stepper_endstop_steps || memoryArray[i].steps < 0) {
                #ifdef DEBUG
                Serial.println("\nselectMemoryByFrequency()");
                Serial.println("Invalid memory! (steps out of range)");
                Serial.println("closestIndex: " + String(i));
                Serial.println("memoryArray[closestIndex].steps:" + String(memoryArray[i].steps));
                #endif
                return;
            }

            currentMemoryIndex = i;
            storeCurrentMemoryIndex(currentMemoryIndex);
            previewMemoryIndex = currentMemoryIndex;
            stepper_programmed_steps = constrain(memoryArray[currentMemoryIndex].steps, 0, stepper_endstop_steps);
            //currentFrequency = memoryArray[currentMemoryIndex].khz;
            currentFrequency = khz;
            #ifdef DEBUG
            Serial.println("New Memory Index: " + String(currentMemoryIndex));
            Serial.println("New Frequency: " + String(currentFrequency));
            Serial.println("New Stepper Programmed Position: " + String(stepper_programmed_steps));
            #endif
            stepper_keep_enabled();
            stepper.moveTo(stepper_programmed_steps);
            preferences.putInt(KHZ_KEY, currentFrequency);
            refreshTuningScreen();
            return;
        }
    }
    #ifdef DEBUG
    Serial.println("No matching memory found");
    #endif
}

void selectMemoryByIndex(u_int16_t index) {
    
    if (memoryArraySize == 0) {
        return;
    }

    #ifdef DEBUG
    Serial.println("\nselectMemoryByIndex()");
    #endif

    if (index < 0 ||
        index >= memoryArraySize) {
        #ifdef DEBUG
        Serial.println("Invalid index!");
        Serial.println("index: " + String(index));       
        #endif
        return;
    }

    if (memoryArray[index].steps > stepper_endstop_steps ||
        memoryArray[index].steps < 0) {
        #ifdef DEBUG
        Serial.println("Invalid memory! (steps out of range)");
        Serial.println("index: " + String(index));
        Serial.println("memoryArray[index].steps:" + String(memoryArray[currentMemoryIndex].steps));
        #endif
        return;
    }

    currentMemoryIndex = index;
    storeCurrentMemoryIndex(currentMemoryIndex);
    previewMemoryIndex = currentMemoryIndex;
    stepper_programmed_steps = constrain(memoryArray[currentMemoryIndex].steps, 0, stepper_endstop_steps);
    
    currentFrequency = memoryArray[currentMemoryIndex].khz;
    
    if (rigctldActive) {
        setFrequencyByRigctld(currentFrequency);
    }

    #ifdef DEBUG
    Serial.println("New Memory Index: " + String(currentMemoryIndex));
    Serial.println("New Frequency: " + String(memoryArray[currentMemoryIndex].khz));
    Serial.println("New Stepper Programmed Position: " + String(stepper_programmed_steps));
    #endif
    stepper_keep_enabled();
    stepper.moveTo(stepper_programmed_steps);
    preferences.putInt(KHZ_KEY, currentFrequency);
    refreshTuningScreen();
}

void addMemory(long steps, u_int32_t khz) {

    #ifdef DEBUG
    Serial.println("\naddMemory()");
    #endif

    // Check for invalid step count
    if (steps >= stepper_endstop_steps) {
        #ifdef DEBUG
        Serial.println("Invalid memory! (steps out of range)");
        Serial.println("steps: " + String(steps));
        Serial.println("stepper_endstop_steps: " + String(stepper_endstop_steps));
        #endif
        return;
    }

    // Check for duplicate khz and update if necessary
    for (size_t i = 0; i < memoryArraySize; ++i) {
        if (memoryArray[i].khz == khz) {
            memoryArray[i].steps = steps;
            #ifdef DEBUG
            Serial.println("\nDuplicate Memory found, updated existing memory!");
            debugPrintMemoryArray();
            #endif
            storeMemories();
            return;
        }
    }

    // Add new memory if no duplicate found
    if (memoryArraySize < MEMORY_MAX_SIZE) {
        memoryArray[memoryArraySize].steps = steps;
        memoryArray[memoryArraySize].khz = khz;
        memoryArraySize++;
    }

    // Sort the array by khz, leaving empty slots at the end
    std::sort(memoryArray, memoryArray + memoryArraySize,
              [](const StepData& a, const StepData& b) {
                  if (a.khz == 0) return false;
                  if (b.khz == 0) return true;
                  return a.khz < b.khz;
              });

    selectMemoryByFrequency(khz);
    storeMemories();
    #ifdef DEBUG
    Serial.println("\nMemory added successfully!");
    Serial.println("Memory Array Size: " + String(memoryArraySize));
    debugPrintMemoryArray();
    #endif
}

void deleteMemory() {
    if (memoryArraySize > 0) {
        // Fill the position to be deleted with zeros to mark it as empty
        memoryArray[currentMemoryIndex].steps = 0;
        memoryArray[currentMemoryIndex].khz = 0;
        // Shift all elements after the deleted position to the left
        for (size_t i = currentMemoryIndex; i < memoryArraySize - 1; ++i) {
            memoryArray[i] = memoryArray[i + 1];
        }
        memoryArraySize--;
        if (currentMemoryIndex >= memoryArraySize) {
            if (currentMemoryIndex > 0) {
                currentMemoryIndex = memoryArraySize - 1;
            }
        }
        storeMemories();
        #ifdef DEBUG
        Serial.println("\nMemory " + String(currentMemoryIndex) + " deleted successfully!");
        debugPrintMemoryArray();
        #endif
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Memory ");
        lcd.print(currentMemoryIndex);
        lcd.setCursor(0, 1);
        lcd.print(" deleted");
        delay(1000);
        // select an adiancent memory
        memoryDown();
        // avoid movement triggered by memory_loop()
        currentMemoryIndex = previewMemoryIndex;
    }
}

void deleteAllMemories() {
    memoryArraySize = 0;
    currentMemoryIndex = 0;
    for (size_t i = 0; i < MEMORY_MAX_SIZE; ++i) {
        memoryArray[i].steps = 0;
        memoryArray[i].khz = 0;
    }
    storeMemories();
    #ifdef DEBUG
    Serial.println("\nAll memories deleted successfully!");
    debugPrintMemoryArray();
    #endif
    lcd.clear();
    lcd.print("ALL MEMORIES");
    lcd.setCursor(0, 1);
    lcd.print("DELETED");
    delay(1000);
}

u_int32_t last_preview_memory_set = 0;

void previewMemoryByIndex(u_int16_t index) {
    last_preview_memory_set = millis();
    printSelectedMemory();
    //printSelectedSteps();
    printFrequency(memoryArray[index].khz);
    previewMemoryIndex = index;
}

void memory_loop() {
    if (currentMemoryIndex != previewMemoryIndex && millis() - last_preview_memory_set > 2000) {
        #ifdef DEBUG
        Serial.println("\nmemory_loop()");
        Serial.println("Selecting Memory: ") + String(previewMemoryIndex + 0);  
        #endif
        selectMemoryByIndex(previewMemoryIndex);
    }
}

void memoryUp() {
    // If there is only one memory, select it
    if (memoryArraySize == 1) {
        previewMemoryByIndex(0);
        return; 
    // If there are no memories, or we are at the last memory, do nothing
    } else if (memoryArraySize == 0 || previewMemoryIndex == memoryArraySize - 1) {
        #ifdef DEBUG
        Serial.println("\nMemory Array Size: " + String(memoryArraySize));
        Serial.println("Preview Memory Index: " + String(previewMemoryIndex));
        #endif
        return;
    // If we are not at the last memory, select the next memory
    } else {
        u_int16_t nextMemoryIndex = previewMemoryIndex + 1;
        previewMemoryByIndex(nextMemoryIndex);
    }   
}

void memoryDown() {
    // If there is only one memory, select it
    if (memoryArraySize == 1) {
        previewMemoryByIndex(0);
        return; 
    // If there are no memories, or we are at the first memory, do nothing
    } else if (memoryArraySize == 0 || previewMemoryIndex == 0) {
        #ifdef DEBUG
        Serial.println("\nMemory Array Size: " + String(memoryArraySize));
        Serial.println("Current Memory Index: " + String(previewMemoryIndex));
        #endif
        return;
    // If we are not at the first memory, select the previous memory
    } else {
        u_int16_t nextMemoryIndex = previewMemoryIndex - 1;
        previewMemoryByIndex(nextMemoryIndex);
    }   
}
