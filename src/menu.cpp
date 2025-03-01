#include "menu.h"

LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, 16, 2);
LcdMenu menu(renderer);

ButtonAdapter upButtonA(&menu, &button3, UP);
ButtonAdapter downButtonA(&menu, &button5, DOWN);
ButtonAdapter enterButtonA(&menu, &button4, ENTER);

bool menuVisible = false;
bool settingFrequency = false;
bool settingEndstop = false;

static const char* mem_methods[] = {"Manual", "Auto"};
static const uint8_t MEM_METHODS_COUNT = sizeof(mem_methods) / sizeof(mem_methods)[0];


MENU_SCREEN(settingsScreen, settingsItems,
  ITEM_COMMAND("Show IP Addr.", []() {
    menu.hide();
    showMessage("IP Address:", WiFi.localIP().toString());
    menu.show();
  }),
  ITEM_COMMAND("Show Version", []() {
    menu.hide();
    showMessage("K1FM QRO Loop", VERSION);
    menu.show();
  }),
  ITEM_COMMAND("Adjust Mode", (void (*)())adjustingSet),
  // ITEM_COMMAND("Delete all Memories", []() {
  //   deleteAllMemories();
  //   menuVisible = false;
  //   menu.hide();
  //   refreshTuningScreen();
  // }),
  ITEM_COMMAND("Set Endstop", []() {
    menuVisible = false;
    menu.hide();
    settingEndstop = true;
    menuVisible = false;
    // unless the upper bound is changed, the rotary
    // encoder will prevent a new, higher endstop
    // from being set therefore we temporarily set
    // the upper bound to UINT16_MAX
    r.setUpperBound(UINT16_MAX);
    r.resetPosition(stepper_endstop_steps);
    menu.hide();
    printEndstop();
  }),
  ITEM_TOGGLE("RadioContr", [](bool isOn) { 
    #ifdef DEBUG
    Serial.print("Selected rigctldActive: ");
    Serial.println(isOn);
    #endif
    setRigctldActive(isOn);
  }),
  ITEM_TOGGLE("AutoMemory", [](bool isOn) { 
    #ifdef DEBUG
    Serial.print("Selected rigctldActive: ");
    Serial.println(isOn);
    #endif
    setAutoMemorySelection(isOn);
  }),
  // ITEM_WIDGET(
  //   "Memory",
  //   [](const uint8_t method) {
  //     #ifdef DEBUG
  //     Serial.print("Selected Memory Method: ");
  //     Serial.println(mem_methods[method]);
  //     #endif
  //     if (method == 0) {
  //       setAutoMemorySelection(false);
  //     } else {
  //       setAutoMemorySelection(true);
  //     }
  //   },
  //   WIDGET_LIST(mem_methods, MEM_METHODS_COUNT, 0, "%s", 0, true)),
  ITEM_BACK("Back"));

MENU_SCREEN(mainScreen, mainItems,
    ITEM_COMMAND("Measure SWR", []() {
      if (!rigctldActive) {
        menu.hide();
        showMessage("RIG CONTROL", "NOT ACTIVE");
        menu.show();
        return;
      }
      menuVisible = false;
      menu.hide();
      lcd.setCursor(0,0);
      lcd.print("MEASURING SWR");
      float SWR = findMinimumSWRByRigctld(true);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("DONE");
      lcd.setCursor(0,1);
      lcd.print("SWR: ");
      lcd.print(String(SWR, 1));
      delay(2000);
      lcd.clear();
      refreshTuningScreen();
    }),
    ITEM_COMMAND("Auto Tune", []() {

      if (!rigctldActive) {
        menu.hide();
        showMessage("RIG CONTROL", "NOT ACTIVE");
        menu.show();
        return;
      }

      menuVisible = false;
      menu.hide();
      lcd.setCursor(0,0);
      lcd.print("TUNING          ");
      float SWR = findMinimumSWRByRigctld();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("DONE TUNING    ");
      lcd.setCursor(0,1);
      lcd.print("SWR: ");
      lcd.print(String(SWR, 1));
      delay(2000);
      lcd.clear();
      refreshTuningScreen();
    }),
    ITEM_COMMAND("Save Memory", []() {
      menuVisible = false;
      menu.hide();
      settingFrequency = true;
      menuVisible = false;
      menu.hide();
      if (rigctldActive) {
        u_int32_t frequency = getFrequencyByRigctld();
        if (frequency != 0) {
          currentFrequency = frequency;
        }
      }
      r.setUpperBound(UINT16_MAX);
      printFrequency(currentFrequency);
    }),
    ITEM_COMMAND("Delete Memory", []() {
      deleteMemory();
      menuVisible = false;
      menu.hide();
      refreshTuningScreen();
    }),
    ITEM_SUBMENU("Settings", settingsScreen),
    ITEM_COMMAND("Exit Menu", []() {
      menuVisible = false;
      menu.hide();
      refreshTuningScreen();
    }));

void menu_loop() {
    static unsigned long lastMenuVisible = millis();

    if (menuVisible && (millis() - lastMenuVisible > 60 * 2 * 1000)) {
      menuVisible = false;
      menu.hide();
      refreshTuningScreen();
      lastMenuVisible = millis();
    }

    upButtonA.observe();
    downButtonA.observe();
    enterButtonA.observe();

}

uint16_t adjustingSet() {
  menuVisible = false;
  menu.hide();
  lcd.clear();
  adjusting = true;
  adjustPosition = stepper.currentPosition();
  refreshTuningScreen();
  return adjustPosition;
}

void setRigctldActive(bool isOn) {
  rigctldActive = isOn;
  preferences.putBool(RIGCTLD_ACTIVE_KEY, rigctldActive);
  if (!rigctldActive) {
    // there cannot be automatic memory selection unless
    // the radio is being controlled
    setAutoMemorySelection(false);
    disconnectFromRigctld();
  }

  MenuItem* setRigctldMenuItem = settingsItems[4];
  ItemToggle* setRigctldItemToggle = static_cast<ItemToggle*>(setRigctldMenuItem);
  setRigctldItemToggle->setIsOn(isOn);
  menu.refresh();

}

void setAutoMemorySelection(bool isOn) {
  automaticMemorySelection = isOn;
  preferences.putBool(AUTOMATIC_MEMORY_SELECTION_KEY, automaticMemorySelection);
  if (automaticMemorySelection) {
    // there cannot be automatic memory selection unless
    // the radio is being controlled
    setRigctldActive(true);
  }

  MenuItem* automaticMemoryMenuItem = settingsItems[5];
  ItemToggle* automaticMemoryMenuItemItemToggle = static_cast<ItemToggle*>(automaticMemoryMenuItem);
  automaticMemoryMenuItemItemToggle->setIsOn(isOn);
  menu.refresh();

}
