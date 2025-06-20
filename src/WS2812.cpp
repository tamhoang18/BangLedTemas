#include "WS2812.h"
#include <Adafruit_NeoPixel.h> // Thư viện điều khiển LED WS2812

#define WS2801_Pin 2                                                   // Chân kết nối với LED WS2812
#define Number_Led 256                                                 // Số lượng LED
Adafruit_NeoPixel strip(Number_Led, WS2801_Pin, NEO_GRB + NEO_KHZ800); // Khởi tạo đối tượng LED WS2812

void InitLedRGY() // hàm khởi tạo Led RGY
{
  strip.begin();
  strip.setBrightness(15);
  strip.show();
}

void ColorGreen() // hàm gọi màu xanh lá
{
  for (int i = 0; i < Number_Led; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
}

void ColorRed() // hàm gọi màu đỏ
{
  for (int i = 0; i < Number_Led; i++)
  {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
}

void ColorYellow() // hàm gọi màu vàng
{
  for (int i = 0; i < Number_Led; i++)
  {
    strip.setPixelColor(i, strip.Color(255, 255, 0));
  }
}

void ShowColor() // hàm gọi màu hiển thị
{
  strip.show();
}

void ColorOff() // hàm gọi tắt màu
{
  for (int i = 0; i < Number_Led; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}
