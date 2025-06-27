#include "WS2812.h"
#include <Adafruit_NeoPixel.h> // Thư viện điều khiển LED WS2812

#define WS2801_Pin 2
#define NUM_LEDS 256
Adafruit_NeoPixel strip(NUM_LEDS, WS2801_Pin, NEO_GRB + NEO_KHZ800);

void Init_LedRGY() // hàm khởi tạo Led RGY
{
  strip.begin();
  strip.setBrightness(15);
  strip.show();
}

void ColorGreen() // Sáng xanh từ led 8 đến 247, các led còn lại tắt
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i >= 8 && i <= 247)
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    else
      strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void ColorRed() // Sáng đỏ từ led 8 đến 247, các led còn lại tắt
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i >= 8 && i <= 247)
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    else
      strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void ColorYellow() // Sáng vàng từ led 8 đến 247, các led còn lại tắt
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i >= 8 && i <= 247)
      strip.setPixelColor(i, strip.Color(255, 255, 0));
    else
      strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void ColorOff() // Tắt toàn bộ dải led
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}