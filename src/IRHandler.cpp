#include "Eeprom.h"
#include "IRHandler.h"
extern void handleExitMenu();

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;
int btn3Count = 0;
unsigned long btn3LastTime = 0;
int btn4Count = 0;
unsigned long btn4LastTime = 0;

MenuState currentMenu = MENU_IDLE;
int currentPosition = 0;
int values[6] = {0, 0, 0, 0, 0, 0};

int getStepAmount(int count)
{
  if (count >= 35) return 50;
  else if (count >= 30) return 25;
  else if (count >= 25) return 20;
  else if (count >= 20) return 15;
  else if (count >= 15) return 10;
  else if (count >= 10) return 5;
  else return 1;
}

const String DS_Nut_Nhan[NUM_BUTTONS] = {
    "6322900E", // Nút 0
    "0C796DFC", // Nút 1
    "DB510F56", // Nút 2
    "7E16B93A", // Nút 3 - tăng giá trị
    "C2A82EEA", // Nút 4 - giảm giá trị
    "8D6B3416"  // Nút 5 - chưa dùng
};

unsigned long lastPressTime = 0;
int pressCount = 0;

// Lọc mã lặp
String Sau_Khi_Doc_Code = "";
unsigned long lastIRTime = 0;

// Buzzer
bool isBeeping = false;
unsigned long beepStartTime = 0;
const int beepDuration = 50;

void startBeep()
{
  digitalWrite(BUZZER_PIN, HIGH);
  isBeeping = true;
  beepStartTime = millis();
}

void updateBeep()
{
  if (isBeeping && millis() - beepStartTime >= beepDuration)
  {
    digitalWrite(BUZZER_PIN, LOW);
    isBeeping = false;
  }
}

String hexToString(uint64_t code)
{
  char buf[9];
  sprintf(buf, "%08lX", (uint32_t)(code & 0xFFFFFFFF));
  return String(buf);
}

void Init_IR()
{
  irrecv.enableIRIn();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void printValues()
{
  Serial.println("------ GIÁ TRỊ HIỆN TẠI ------");
  for (int i = 0; i < 6; i++)
  {
    Serial.printf("Vị trí %d: %02d\n", i + 1, values[i]);
  }
  Serial.println("------------------------------");
}

void handleIR()
{
  if (irrecv.decode(&results))
  {
    String Bien_Doc_Code = hexToString(results.value);
    unsigned long now = millis();

    // Lọc mã lặp IR: nếu giống mã trước và trong 200ms thì bỏ qua
    if (Bien_Doc_Code == Sau_Khi_Doc_Code && (now - lastIRTime < 200)) {
      irrecv.resume();
      return;
    }
    Sau_Khi_Doc_Code = Bien_Doc_Code;
    lastIRTime = now;

    Serial.print("Mã IR nhận được: ");
    Serial.println(Bien_Doc_Code);
    startBeep();

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
      if (Bien_Doc_Code == DS_Nut_Nhan[i])
      {
        if (now - lastPressTime > PRESS_TIMEOUT)
          pressCount = 0;
        lastPressTime = now;

        switch (i)
        {
        case 0: // Nút 0
          pressCount++;
          Serial.printf("Nút 0 nhấn %d lần\n", pressCount);
          if (pressCount >= 10)
          {
            if (currentMenu == MENU_IDLE)
            {
              currentMenu = MENU_SELECT_POSITION;
              currentPosition = 0;
              Serial.println(">>> ĐÃ VÀO MENU (SELECT_POSITION) <<<");
            }
            else
            {
              handleExitMenu();
              Serial.println(">>> THOÁT MENU <<<");
            }
            pressCount = 0;
            printValues();
          }
          break;

        case 1: // Nút 1 → Chọn vị trí tăng
          if (currentMenu == MENU_BLINK || currentMenu == MENU_SELECT_POSITION)
          {
            currentMenu = MENU_SELECT_POSITION;
            currentPosition = (currentPosition + 1) % 6;
            Serial.printf("Chọn vị trí tăng: %d\n", currentPosition + 1);
          }
          break;

        case 2: // Nút 2 → Chọn vị trí giảm
          if (currentMenu == MENU_BLINK || currentMenu == MENU_SELECT_POSITION)
          {
            currentMenu = MENU_SELECT_POSITION;
            currentPosition = (currentPosition - 1 + 6) % 6;
            Serial.printf("Chọn vị trí giảm: %d\n", currentPosition + 1);
          }
          break;

        case 3: // Nút 3 → tăng giá trị
          if (currentMenu == MENU_SELECT_POSITION)
          {
            if (now - btn3LastTime > 500)
              btn3Count = 0;
            btn3Count++;
            btn3LastTime = now;
            btn4Count = 0;

            int delta = getStepAmount(btn3Count);
            values[currentPosition] += delta;

            if (currentPosition <= 3)
              values[currentPosition] = constrain(values[currentPosition], 0, 999);
            else
              values[currentPosition] = constrain(values[currentPosition], 0, 9999);

            Serial.printf("Nút 3 [+%d] → vị trí %d = %d (nhấn %d lần)\n",
                          delta, currentPosition + 1, values[currentPosition], btn3Count);
          }
          break;
        case 4: // Nút 4 → giảm giá trị
          if (currentMenu == MENU_SELECT_POSITION)
          {
            if (now - btn4LastTime > 500)
              btn4Count = 0;
            btn4Count++;
            btn4LastTime = now;
            btn3Count = 0;

            int delta = getStepAmount(btn4Count);
            values[currentPosition] -= delta;

            if (currentPosition <= 3)
              values[currentPosition] = constrain(values[currentPosition], 0, 999);
            else
              values[currentPosition] = constrain(values[currentPosition], 0, 9999);

            Serial.printf("Nút 4 [-%d] → vị trí %d = %d (nhấn %d lần)\n",
                          delta, currentPosition + 1, values[currentPosition], btn4Count);
          }
          break;

        default:
          break;
        }
        break;
      }
    }

    irrecv.resume();
  }
}