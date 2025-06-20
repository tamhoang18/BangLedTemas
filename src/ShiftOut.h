#pragma once
#include <Arduino.h>
#include <RTClib.h>

#define Data_ShiftOurt 14
#define Clock_ShiftOut 27
#define Latch_ShiftOut 12
#define Number_ShiftOut 39

extern byte Led_Data[Number_ShiftOut];

void shiftOut4094(byte *data, int length);
void updateDisplayData(float temp, float humi, DateTime now, bool blink);
void displayValueAtPosition(int position, int value);
void blinkOtherPositions(int fixedPosition, bool on);
extern const byte PositionLEDs[6][4];
void InitShiftOut();
void clearOtherPositions(int fixedPosition);