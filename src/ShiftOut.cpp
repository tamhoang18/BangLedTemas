#include "ShiftOut.h"

byte Led_Data[Number_ShiftOut];

const byte Digit_To_7Seg[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b10000000, // .
    0b01000000  // -
};

void InitShiftOut()
{
    pinMode(Data_ShiftOurt, OUTPUT);
    pinMode(Clock_ShiftOut, OUTPUT);
    pinMode(Latch_ShiftOut, OUTPUT);
}

void shiftOutSlow(byte data)
{
    for (int i = 7; i >= 0; i--)
    {
        digitalWrite(Clock_ShiftOut, LOW);

        digitalWrite(Data_ShiftOurt, (data >> i) & 1);
        delayMicroseconds(10);

        digitalWrite(Clock_ShiftOut, HIGH);
        delayMicroseconds(10);
    }
}

void shiftOut4094(byte *data, int length)
{
    digitalWrite(Latch_ShiftOut, LOW);
    for (int i = length - 1; i >= 0; i--)
    {
        shiftOutSlow(data[i]);
    }
    digitalWrite(Latch_ShiftOut, HIGH);
}
const byte PositionLEDs[6][4] = {
    {3, 4, 5, 0},     // vị trí 1
    {7, 8, 9, 0},     // vị trí 2
    {13, 14, 15, 0},  // vị trí 3
    {17, 18, 19, 0},  // vị trí 4
    {30, 31, 32, 33}, // vị trí 5
    {35, 36, 37, 38}  // vị trí 6
};

void displayValueAtPosition(int position, int value)
{
    if (position < 0 || position >= 6)
        return;

    const byte *leds = PositionLEDs[position];

    switch (position)
    {
    case 0: // Vị trí 1 → 00.0 (3 LED)
    case 1: // Vị trí 2 → 00.0
    case 2: // Vị trí 3 → 00.0
    case 3: // Vị trí 4 → 00.0
    {
        value = constrain(value, 0, 999);
        byte d1 = (value / 100) % 10;
        byte d2 = (value / 10) % 10;
        byte d3 = value % 10;

        Led_Data[leds[0]] = ~Digit_To_7Seg[d1];
        Led_Data[leds[1]] = ~(Digit_To_7Seg[d2] | 0x80);
        Led_Data[leds[2]] = ~Digit_To_7Seg[d3]; // DP tại số cuối
        break;
    }

    case 4: // Vị trí 5 → 000.0 (4 LED)
    case 5: // Vị trí 6 → 000.0
    {
        value = constrain(value, 0, 9999);
        byte d1 = (value / 1000) % 10;
        byte d2 = (value / 100) % 10;
        byte d3 = (value / 10) % 10;
        byte d4 = value % 10;

        Led_Data[leds[0]] = Digit_To_7Seg[d1];
        Led_Data[leds[1]] = Digit_To_7Seg[d2];
        Led_Data[leds[2]] = ((Digit_To_7Seg[d3]) | 0x80);
        Led_Data[leds[3]] = Digit_To_7Seg[d4]; // DP tại số cuối
        break;
    }

    default:
        break;
    }
    // 👉 Thêm dấu trang trí hoặc dấu trừ cố định
    Led_Data[6] = ~Digit_To_7Seg[11];
    Led_Data[16] = ~Digit_To_7Seg[11];
    Led_Data[34] = Digit_To_7Seg[11];
}

void clearOtherPositions(int fixedPosition)
{
    for (int i = 0; i < 6; i++)
    {
        if (i == fixedPosition)
            continue;
        const byte *leds = PositionLEDs[i];

        int count = (i <= 3) ? 3 : 4; // vị trí 1–4 dùng 3 LED, 5–6 dùng 4 LED

        for (int j = 0; j < count; j++)
        {
            Led_Data[leds[j]] = 0xFF;
        }
    }
}

void blinkOtherPositions(int fixedPosition, bool on)
{
    for (int i = 0; i < 6; i++)
    {
        if (i == fixedPosition)
            continue;

        const byte *leds = PositionLEDs[i];
        int count = (i <= 3) ? 3 : 4;

        for (int j = 0; j < count; j++)
        {
            Led_Data[leds[j]] = on ? 0x00 : 0xFF;
        }
    }
}

void updateDisplayData(float temp, float humi, DateTime now, bool blink)
{
    for (int i = 0; i < Number_ShiftOut; i++)
        Led_Data[i] = 0xFF;

    int tempInt = (int)(temp * 100);
    int humiInt = (int)(humi * 100);

    // Nhiệt độ: LED 0-7
    Led_Data[0] = Digit_To_7Seg[(tempInt / 1000) % 10];
    Led_Data[1] = (Digit_To_7Seg[(tempInt / 100) % 10] | Digit_To_7Seg[10]);
    Led_Data[2] = Digit_To_7Seg[(tempInt / 10) % 10];

    // // Dải trang trí nhiệt độ
    // Led_Data[3] = ~0b01011011;
    // Led_Data[4] = ~(0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[5] = ~0b00111111;
    // Led_Data[6] = ~Digit_To_7Seg[11]; // dấu -
    // Led_Data[7] = ~0b01100110;
    // Led_Data[8] = ~(0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[9] = ~0b00111111;

    // Độ ẩm: LED 10-12
    Led_Data[10] = Digit_To_7Seg[(humiInt / 1000) % 10];
    Led_Data[11] = (Digit_To_7Seg[(humiInt / 100) % 10] | Digit_To_7Seg[10]);
    Led_Data[12] = Digit_To_7Seg[(humiInt / 10) % 10];

    // // Dải trang trí độ ẩm
    // Led_Data[13] = ~0b01101101;
    // Led_Data[14] = ~(0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[15] = ~0b00111111;
    // Led_Data[16] = ~Digit_To_7Seg[11];
    // Led_Data[17] = ~0b01111101;
    // Led_Data[18] = ~(0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[19] = ~0b00111111;

    // Ngày tháng năm: LED 20-25
    Led_Data[20] = ~Digit_To_7Seg[now.day() / 10];
    Led_Data[21] = ~Digit_To_7Seg[now.day() % 10];
    Led_Data[22] = ~Digit_To_7Seg[now.month() / 10];
    Led_Data[23] = ~Digit_To_7Seg[now.month() % 10];
    Led_Data[24] = ~Digit_To_7Seg[(now.year() % 100) / 10];
    Led_Data[25] = ~Digit_To_7Seg[(now.year() % 100) % 10];

    // Giờ: nhấp nháy dấu `.`
    if (blink)
    {
        Led_Data[26] = ~(Digit_To_7Seg[now.hour() / 10] | Digit_To_7Seg[10]); // có chấm
        Led_Data[27] = ~(Digit_To_7Seg[now.hour() % 10] | Digit_To_7Seg[10]); // có chấm
    }
    else
    {
        Led_Data[26] = ~Digit_To_7Seg[now.hour() / 10];
        Led_Data[27] = ~Digit_To_7Seg[now.hour() % 10];
    }
    Led_Data[28] = ~Digit_To_7Seg[now.minute() / 10];
    Led_Data[29] = ~Digit_To_7Seg[now.minute() % 10];

    // Dải LED trang trí còn lại
    // Led_Data[30] = 0b01011011;
    // Led_Data[31] = 0b00111111;
    // Led_Data[32] = (0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[33] = 0b00111111;
    // Led_Data[34] = Digit_To_7Seg[11];
    // Led_Data[35] = 0b00111111;
    // Led_Data[36] = 0b01100110;
    // Led_Data[37] = (0b00111111 | Digit_To_7Seg[10]);
    // Led_Data[38] = 0b00111111;
}
