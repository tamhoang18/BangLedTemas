#pragma once
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS  256
#define WS2801_Pin 2

extern Adafruit_NeoPixel strip;  // <-- Cho phép main.cpp truy cập strip

void Init_LedRGY();
void ColorGreen();
void ColorRed();
void ColorYellow();
void ShowColor();
void ColorOff();

