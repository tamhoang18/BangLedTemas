#include <Arduino.h>
#include "ShiftOut.h"
#include "DS3231.h"
#include "RS485.h"
#include "IRHandler.h"
#include "Eeprom.h"
#include "WS2812.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT 5 // WDT timeout in seconds
unsigned long lastUpdate485 = 0;
unsigned long lastClockBlink = 0;
bool blinkClock = false;

unsigned long lastMenuBlink = 0;
bool blinkOn = true;
const unsigned long blinkInterval = 500;

void clearPosition(int pos)
{
  const byte *leds = PositionLEDs[pos];
  int len = (pos < 4) ? 3 : 4;
  for (int i = 0; i < len; i++)
  {
    // Nếu là vị trí 5 hoặc 6, đảo ngược bit do ULN2803
    Led_Data[leds[i]] = (pos >= 4) ? 0x00 : 0xFF;
  }
}

void handleExitMenu()
{
  currentMenu = MENU_IDLE;
  saveValuesToEEPROM(values, 6); // Lưu EEPROM khi thoát
  Serial.println(">>> ĐÃ THOÁT MENU và ĐÃ LƯU GIÁ TRỊ EEPROM <<<");
  ColorGreen();
  ShowColor();
  delay(500);
  ColorOff();
  ShowColor();
}
// int melody[] = {
//     262, 262, 0, 262, 0, 196, 262, 0, 0,
//     196, 0, 196, 220, 0, 247, 0, 233, 220,
//     196, 262, 0, 0};

// int noteDurations[] = {
//     200, 200, 150, 200, 150, 400, 200, 150, 150,
//     400, 150, 200, 200, 150, 200, 150, 200, 200,
//     500, 400, 150, 150};
// void playStartupMelody()
// {
//   int notes = sizeof(melody) / sizeof(melody[0]);

//   for (int i = 0; i < notes; i++)
//   {
//     int noteDuration = noteDurations[i];
//     if (melody[i] == 0)
//     {
//       noTone(BUZZER_PIN); // nghỉ
//     }
//     else
//     {
//       tone(BUZZER_PIN, melody[i], noteDuration);
//     }
//     delay(noteDuration + 50); // thêm khoảng nghỉ giữa các nốt
//   }

//   noTone(BUZZER_PIN);
// }

void setup()
{
  Serial.begin(115200);
  // pinMode(BUZZER_PIN, OUTPUT);
  // playStartupMelody(); // Phát khi khởi động
  InitLedRGY();
  ColorOff();   // Tắt tất cả LED
  strip.show(); // Bắt buộc gọi show() sau khi thay đổi màu
  InitRTC();
  InitRS485();
  InitShiftOut();
  initEEPROM();
  loadValuesFromEEPROM(values, 6); // Tải giá trị đã lưu trước đó

  setupIR(); // IR receiver init

  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL); // Gắn WDT vào task chính
}

void loop()
{
  esp_task_wdt_reset(); // Reset watchdog
  unsigned long now = millis();

  // Cập nhật nhiệt độ + độ ẩm từ RS485 mỗi 2 giây
  if (now - lastUpdate485 >= 2000)
  {
    lastUpdate485 = now;
    updateModbus();
  }

  // Xử lý remote IR
  handleIR();

  // Nhấp nháy dấu chấm ở đồng hồ (dù ở chế độ nào)
  if (now - lastClockBlink >= 500)
  {
    lastClockBlink = now;
    blinkClock = !blinkClock;
    DateTime nowTime = getTime();
    updateDisplayData(getTemp(), getHumi(), nowTime, blinkClock);
  }

  // Nhấp nháy LED theo menu nếu đang ở menu
  if (currentMenu != MENU_IDLE && now - lastMenuBlink >= blinkInterval)
  {
    lastMenuBlink = now;
    blinkOn = !blinkOn;
  }

  // Hiển thị dữ liệu LED tại 6 vị trí
  for (int i = 0; i < 6; i++)
  {
    if (currentMenu == MENU_SELECT_POSITION)
    {
      if (i == currentPosition && !blinkOn)
        clearPosition(i);
      else
        displayValueAtPosition(i, values[i]);
    }
    else if (currentMenu == MENU_BLINK)
    {
      // Vào menu thì vị trí 1 nhấp nháy để biết đã vào
      if (i == 0 && !blinkOn)
        clearPosition(0);
      else
        displayValueAtPosition(i, values[i]);
    }
    else
    {
      // Bình thường
      displayValueAtPosition(i, values[i]);
    }
  }

  // Gửi dữ liệu ra dãy LED qua IC 4094
  updateBeep();
  shiftOut4094(Led_Data, Number_ShiftOut);
}
