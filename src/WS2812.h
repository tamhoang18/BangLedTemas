#ifndef WS2812_H
#define WS2812_H

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS  256
#define WS2801_Pin 2

extern Adafruit_NeoPixel strip;  // <-- Cho phép main.cpp truy cập strip

void InitLedRGY();
void ColorGreen();
void ColorRed();
void ColorYellow();
void ShowColor();
void ColorOff();

#endif
