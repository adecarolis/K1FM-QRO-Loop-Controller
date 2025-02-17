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

uint16_t adjustingSet() {
  menuVisible = false;
  menu.hide();
  lcd.clear();
  adjusting = true;
  adjustPosition = stepper.currentPosition();
  refreshTuningScreen();
  return adjustPosition;
}

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
  }),  ITEM_BACK("Back"));


MENU_SCREEN(mainScreen, mainItems,
    ITEM_COMMAND("Save Memory", []() {
      menuVisible = false;
      menu.hide();
      settingFrequency = true;
      menuVisible = false;
      menu.hide();
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
    upButtonA.observe();
    downButtonA.observe();
    enterButtonA.observe();   
}