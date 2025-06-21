#include "Eeprom.h"

void initEEPROM()
{
    Wire.begin();
}

void writeEEPROM(int address, byte data)
{
    Wire.beginTransmission(EEPROM_I2C_ADDRESS | ((address >> 8) & 0x07)); // 24C16 có 8 trang
    Wire.write((byte)(address & 0xFF));                                   // địa chỉ byte trong trang
    Wire.write(data);
    Wire.endTransmission();
    delay(5); // đợi EEPROM ghi xong
}

byte readEEPROM(int address)
{
    Wire.beginTransmission(EEPROM_I2C_ADDRESS | ((address >> 8) & 0x07));
    Wire.write((byte)(address & 0xFF));
    Wire.endTransmission();

    Wire.requestFrom((EEPROM_I2C_ADDRESS | ((address >> 8) & 0x07)), 1);
    if (Wire.available())
    {
        return Wire.read();
    }
    return 0;
}

void saveValuesToEEPROM(const int *values, int size)
{
    for (int i = 0; i < size; i++)
    {
        int val = values[i];
        writeEEPROM(i * 2, val >> 8);       // High byte
        writeEEPROM(i * 2 + 1, val & 0xFF); // Low byte
    }
}

void loadValuesFromEEPROM(int *values, int size)
{
    for (int i = 0; i < size; i++)
    {
        byte high = readEEPROM(i * 2);
        byte low = readEEPROM(i * 2 + 1);
        values[i] = (high << 8) | low;
    }
}
