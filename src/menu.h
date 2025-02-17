#ifndef MENU_H
#define MENU_H

#include <LcdMenu.h>
#include <MenuScreen.h>
#include <ItemCommand.h>
#include <ItemBack.h>
#include <ItemWidget.h>
#include <ItemSubMenu.h>
#include <input/ButtonAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <display/LiquidCrystal_I2CAdapter.h>
#include "debug.h"
#include "lcd.h"
#include "rotary.h"
#include "memories.h"
#include "buttons.h"
#include "rotary.h"
#include "version.h"

extern MenuScreen* mainScreen;

extern LiquidCrystal_I2CAdapter lcdAdapter;
extern CharacterDisplayRenderer renderer;
extern LcdMenu menu;

extern bool menuVisible;
extern bool settingFrequency;
extern bool settingEndstop;

uint16_t adjustingSet();
void menu_loop();

#endif