#include "IRHandler.h"

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

MenuState currentMenu = MENU_IDLE;
int currentPosition = 0;
int values[6] = {0, 0, 0, 0, 0, 0};

const String buttonKeys[NUM_BUTTONS] = {
    "6322900E", // Nút 0
    "0C796DFC", // Nút 1
    "DB510F56", // Nút 2
    "7E16B93A", // Nút 3 - tăng
    "C2A82EEA", // Nút 4 - giảm
    "8D6B3416"  // Nút 5 - chưa dùng
};

unsigned long lastPressTime = 0;
int pressCount = 0;

// Biến xử lý nhấn giữ
bool holdActive = false;
int holdButton = -1;
unsigned long holdStartTime = 0;
unsigned long lastChangeTime = 0;

String hexToString(uint64_t code)
{
  char buf[9];
  sprintf(buf, "%08lX", (uint32_t)(code & 0xFFFFFFFF));
  return String(buf);
}

void setupIR()
{
  irrecv.enableIRIn();
  Serial.println("IR Receiver ready...");
}

void handleIR()
{
  if (irrecv.decode(&results))
  {
    String codeStr = hexToString(results.value);
    Serial.print("Mã IR nhận được: ");
    Serial.println(codeStr);

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
      if (codeStr == buttonKeys[i])
      {
        unsigned long now = millis();
        if (now - lastPressTime > PRESS_TIMEOUT)
          pressCount = 0;
        lastPressTime = now;

        if (i == 0)
        {
          pressCount++;
          Serial.printf("Nút 0 nhấn %d lần\n", pressCount);
          if (pressCount >= 20)
          {
            currentMenu = (currentMenu != MENU_SELECT_POSITION) ? MENU_BLINK : MENU_IDLE;
            Serial.println(">>> CHUYỂN TRẠNG THÁI MENU <<<");
          }
          holdActive = false;
        }
        else if (i == 1 && currentMenu == MENU_BLINK)
        {
          currentMenu = MENU_SELECT_POSITION;
          currentPosition = (currentPosition + 1) % 6;
          Serial.printf("Chọn vị trí tăng: %d\n", currentPosition + 1);
          holdActive = false;
        }
        else if (i == 2 && currentMenu == MENU_BLINK)
        {
          currentMenu = MENU_SELECT_POSITION;
          currentPosition = (currentPosition - 1 + 6) % 6;
          Serial.printf("Chọn vị trí giảm: %d\n", currentPosition + 1);
          holdActive = false;
        }
        else if ((i == 3 || i == 4) && currentMenu == MENU_SELECT_POSITION)
        {
          holdActive = true;
          holdButton = i;
          holdStartTime = now;
          lastChangeTime = 0;
        }

        break;
      }
    }
    irrecv.resume();
  }
}

void handleHold()
{
  if (!holdActive || currentMenu != MENU_SELECT_POSITION)
    return;

  unsigned long now = millis();
  unsigned long heldDuration = now - holdStartTime;

  unsigned long interval = 400;
  if (heldDuration > 3000)
    interval = 80;
  else if (heldDuration > 2000)
    interval = 120;
  else if (heldDuration > 1000)
    interval = 200;

  if (now - lastChangeTime >= interval)
  {
    if (holdButton == 3)
    {
      values[currentPosition]++;
    }
    else if (holdButton == 4)
    {
      values[currentPosition]--;
    }

    values[currentPosition] = constrain(values[currentPosition], 0, 99);
    Serial.printf("Vị trí %d: %d\n", currentPosition + 1, values[currentPosition]);
    lastChangeTime = now;
  }
}
