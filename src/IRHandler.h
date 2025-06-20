#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define IR_RECEIVE_PIN 15
#define NUM_BUTTONS 6
#define PRESS_TIMEOUT 2000

enum MenuState {
  MENU_IDLE,
  MENU_BLINK,
  MENU_SELECT_POSITION
};

extern MenuState currentMenu;
extern int currentPosition;
extern int values[6];

void setupIR();
void handleIR();
void handleHold();

#endif
