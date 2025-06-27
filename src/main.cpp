#include <Arduino.h>
#include <algorithm>
#include "ShiftOut.h"
#include "DS3231.h"
#include "RS485.h"
#include "IRHandler.h"
#include "ATCommand.h"
#include "WS2812.h"
#include "MyEEPROM.h"
#include "esp_task_wdt.h"

using std::swap;

#define WDT_TIMEOUT 5
#define AT_BUFFER_SIZE 128

char atBuffer[AT_BUFFER_SIZE];
int atLen = 0;

MyEEPROM eeprom_external(16, 0x50); // EEPROM ngoài (24Cxx), I2C address 0x50
MyEEPROM eeprom_internal(512);      // EEPROM nội bộ (giả lập flash, tối đa 512 byte)

unsigned long lastUpdate485 = 0;
unsigned long lastClockBlink = 0;
bool blinkClock = false;

unsigned long lastMenuBlink = 0;
bool blinkOn = true;
const unsigned long blinkInterval = 500;

void saveValuesToEEPROM(int *values, int len)
{
  for (int i = 0; i < len; i++)
  {
    byte hi = (values[i] >> 8) & 0xFF;
    byte lo = values[i] & 0xFF;
    eeprom_external.write_1_byte(i * 2, hi);
    eeprom_external.write_1_byte(i * 2 + 1, lo);
  }
}

void loadValuesFromEEPROM(int *values, int len)
{
  for (int i = 0; i < len; i++)
  {
    byte hi = eeprom_external.read_1_byte(i * 2);
    byte lo = eeprom_external.read_1_byte(i * 2 + 1);
    values[i] = (hi << 8) | lo;
  }
}

// ===== Tra bảng Hs theo nhiệt độ =====
float getSaturationHumidity(int temp)
{
  const float hsTable[] = {
      0.014690, 0.015648, 0.016662, 0.017734, 0.018869, 0.020070,
      0.021339, 0.022682, 0.024100, 0.025600, 0.027184, 0.028856,
      0.030624, 0.032491, 0.034462, 0.036545, 0.038743, 0.041064,
      0.043514, 0.046101, 0.048833, 0.051720, 0.054760, 0.057980,
      0.061380, 0.064970, 0.068760, 0.072770, 0.076990, 0.081470,
      0.086200, 0.091220, 0.096520, 0.102140, 0.108080, 0.114370};
  if (temp < 20)
    temp = 20;
  if (temp > 55)
    temp = 55;
  return hsTable[temp - 20];
}

void clearPosition(int pos)
{
  const byte *leds = PositionLEDs[pos];
  int len = (pos < 4) ? 3 : 4;
  for (int i = 0; i < len; i++)
    Led_Data[leds[i]] = (pos >= 4) ? 0x00 : 0xFF;
}

void handleExitMenu()
{
  currentMenu = MENU_IDLE;
  saveValuesToEEPROM(values, 6);
  Serial.println(">>> ĐÃ THOÁT MENU và ĐÃ LƯU GIÁ TRỊ EEPROM <<<");
  ColorGreen();
}

void checkEnvironmentWarning(float temp, float humi)
{
  enum WarningLevel
  {
    NORMAL,
    WARNING_YELLOW,
    WARNING_RED
  };
  static WarningLevel lastLevel = NORMAL;
  WarningLevel level = NORMAL;

  float minTemp = values[0] / 10.0, maxTemp = values[1] / 10.0;
  float minHumi = values[2] / 10.0, maxHumi = values[3] / 10.0;

  if (minTemp > maxTemp)
    swap(minTemp, maxTemp);
  if (minHumi > maxHumi)
    swap(minHumi, maxHumi);

  // Kiểm tra nhiệt độ và độ ẩm trong ngưỡng cho phép
  bool temp_in_range = (temp >= minTemp && temp <= maxTemp);
  bool humi_in_range = (humi >= minHumi && humi <= maxHumi);

  float humi_error = 0.0;

  // Nếu độ ẩm nằm ngoài ngưỡng thì tính tỉ lệ sai số
  if (humi < minHumi)
    humi_error = (minHumi - humi) / minHumi * 100.0;
  else if (humi > maxHumi)
    humi_error = (humi - maxHumi) / maxHumi * 100.0;
  else
    humi_error = 0.0;

  if (temp_in_range && humi_in_range)
  {
    level = NORMAL; // Xanh lá
  }
  else
  {
    if (humi_error > 5.0)
      level = WARNING_RED; // Đèn đỏ
    else if (humi_error >= 1.0)
      level = WARNING_YELLOW; // Đèn vàng
    else
      level = WARNING_RED; // Ngoài vùng (nhưng sai số nhỏ hơn 1%) vẫn báo đỏ
  }

  if (level != lastLevel)
  {
    if (level == WARNING_RED)
      ColorRed();
    else if (level == WARNING_YELLOW)
      ColorYellow();
    else
      ColorGreen();
    lastLevel = level;
  }
}

void PrintSystemInfo()
{
  Serial.println("\r\n______________________________");
  Serial.println("*** ESVN-QMS IOT SOLUTION ***");
  Serial.printf("-CHIP Model           : %s Rev: %u\r\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("-CHIP EFuse MAC       : %llX\r\n", ESP.getEfuseMac());
  Serial.printf("-Flash Size           : %u bytes\r\n", ESP.getFlashChipSize());
  Serial.printf("-XTAL Frequency       : %u MHz\r\n", getXtalFrequencyMhz());
  Serial.printf("-APB Frequency        : %u Hz\r\n", getApbFrequency());
  Serial.printf("-CPU Frequency        : %u MHz\r\n", getCpuFrequencyMhz());
  Serial.println("------------------------------");
}

void playStartupMelody()
{
  int melody[] = {262, 262, 0, 262, 0, 196, 262, 0, 0, 196, 0, 196, 220, 0, 247, 0, 233, 220, 196, 262, 0, 0};
  int noteDurations[] = {200, 200, 150, 200, 150, 400, 200, 150, 150, 400, 150, 200, 200, 150, 200, 150, 200, 200, 500, 400, 150, 150};
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    if (melody[i] == 0)
      noTone(BUZZER_PIN);
    else
      tone(BUZZER_PIN, melody[i], noteDurations[i]);
    delay(noteDurations[i] + 50);
  }
  noTone(BUZZER_PIN);
}

void TaskSensor(void *param)
{
  while (1)
  {
    esp_task_wdt_reset();
    if (millis() - lastUpdate485 >= 1000)
    {
      lastUpdate485 = millis();
      updateModbus();
      int temp = round(getTemp());
      int humi = round(getHumi());
      float hs = getSaturationHumidity(temp);
      int dryingSpeed = round(hs * (100.0 - humi) / 100.0 * 10000);
      Serial.printf("Nhiệt độ: %d °C - Độ ẩm: %d %% - Hs: %.6f - Tốc độ khô: %d\n", temp, humi, hs, dryingSpeed);
      checkEnvironmentWarning(temp, humi);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskDisplay(void *param)
{
  while (1)
  {
    unsigned long now = millis();
    if (now - lastClockBlink >= 500)
    {
      lastClockBlink = now;
      blinkClock = !blinkClock;
      updateDisplayData(getTemp(), getHumi(), getTime(), blinkClock);
    }
    if (currentMenu != MENU_IDLE && now - lastMenuBlink >= blinkInterval)
    {
      lastMenuBlink = now;
      blinkOn = !blinkOn;
    }
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
        if (i == 0 && !blinkOn)
          clearPosition(0);
        else
          displayValueAtPosition(i, values[i]);
      }
      else
      {
        displayValueAtPosition(i, values[i]);
      }
    }
    updateBeep();
    shiftOut4094(Led_Data, Number_ShiftOut);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Wire.begin(); // === THÊM DÒNG NÀY ĐẦU TIÊN ===

  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  playStartupMelody();
  PrintSystemInfo();
  Init_LedRGY();
  loadValuesFromEEPROM(values, 6); // Đọc giá trị từ EEPROM ngoài
  // initATCommand();
  eeprom_internal.begin(); // Khởi tạo EEPROM nội bộ
  eeprom_external.begin(); // Khởi tạo EEPROM ngoài
  Init_RTC();
  InitRS485();
  Init_ShiftOut();
  Init_IR();
  esp_task_wdt_init(WDT_TIMEOUT, true);
  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 8192, NULL, 1, NULL, 1);
}

void loop()
{
  handleIR();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  while (Serial.available())
  {
    char ch = Serial.read();
    if (atLen < AT_BUFFER_SIZE - 1)
    {
      atBuffer[atLen++] = ch;
    }
    if (ch == '\n' || ch == '\r')
    {
      atBuffer[atLen] = '\0';
      if (atLen > 1)
      {
        Command_Process(Serial, atBuffer, atLen);
      }
      atLen = 0;
    }
  }
}