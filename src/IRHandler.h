#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define IR_RECEIVE_PIN 34     // Thay bằng chân bạn dùng
#define NUM_BUTTONS 6
#define PRESS_TIMEOUT 2000
#define BUZZER_PIN 25

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
void printValues();
void startBeep();
void updateBeep();
#endif
