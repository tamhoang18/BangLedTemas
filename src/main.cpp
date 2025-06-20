#include <Arduino.h>
#include "ShiftOut.h"
#include "DS3231.h"
#include "RS485.h"
#include "IRHandler.h"

#include "esp_task_wdt.h"
#define WDT_TIMEOUT 5 // Timeout in seconds

unsigned long lastUpdate485 = 0;
unsigned long lastBlink = 0;

bool blinkState = false;
void setup()
{
  Serial.begin(115200);
  Serial.println("ESP32 Started...");
  InitRTC();
  InitRS485();
  InitShiftOut();
  setupIR(); // Bắt đầu IR
  pinMode(0, INPUT_PULLUP);
  esp_task_wdt_init(WDT_TIMEOUT, true); // true = reset WDT khi esp_task_wdt_reset() được gọi
  esp_task_wdt_add(NULL);               // NULL = task hiện tại (loop)
}

void loop()
{
  // if (millis() - lastBlink >= 500)
  // {
  //   lastBlink = millis();
  //   blinkState = !blinkState;

  //   DateTime now = getTime();
  //   updateDisplayData(getTemp(), getHumi(), now, blinkState);
  //   shiftOut4094(Led_Data, Number_ShiftOut);
  // }

  // if (millis() - lastUpdate485 >= 2000)
  // {
  //   lastUpdate485 = millis();
  //   updateModbus(); // cập nhật temp và humi từ cảm biến RS485
  //   // Serial.printf("[MODBUS] Temp: %.2f °C, Humi: %.2f %%\n", getTemp(), getHumi());
  // }
  handleIR();
  handleHold();
  esp_task_wdt_reset();
}

// handleIR();
// if (currentMenu == MENU_SELECT_POSITION)
// {
//   blinkAllSegments(); // chỉ nháy khi đang ở chế độ chọn vị trí
// }
// if (0 == digitalRead(0)) // Kiểm tra nút nhấn
// {
//   while (1)
//   {
//   }
// }

// if (currentMenu == MENU_BLINK)
// {
//   for (int i = 0; i < 6; i++)
//   {
//     displayValueAtPosition(i, values[i]);
//   }
// }
// else if (currentMenu == MENU_SELECT_POSITION)
// {
//   displayValueAtPosition(currentPosition, values[currentPosition]);
//   clearOtherPositions(currentPosition);
// }
// else
// {
//   clearOtherPositions(-1); // Tắt tất cả
// }

// if (holdActive && (millis() - holdStartTime > HOLD_REPEAT_DELAY))
// {
//   static unsigned long lastRepeat = 0;
//   if (millis() - lastRepeat > HOLD_REPEAT_RATE)
//   {
//     values[currentPosition] += (holdButton == 3 ? 1 : -1);
//     lastRepeat = millis();
//   }
// }
// shiftOut4094(Led_Data, Number_ShiftOut);
