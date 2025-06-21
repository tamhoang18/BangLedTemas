#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include <Arduino.h>  // <-- thêm dòng này để định nghĩa kiểu byte
#include <Wire.h>

#define EEPROM_I2C_ADDRESS 0x50  // Địa chỉ I2C của 24C16

void initEEPROM();
void writeEEPROM(int address, byte data);
byte readEEPROM(int address);
void saveValuesToEEPROM(const int* values, int size);
void loadValuesFromEEPROM(int* values, int size);

#endif
