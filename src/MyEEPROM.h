// File: MyEEPROM.h - Khai báo lớp quản lý EEPROM nội bộ và EEPROM ngoài (IC 24Cxx)

#ifndef MY_EEPROM_H
#define MY_EEPROM_H

#include <Arduino.h>
#include <Wire.h>   // Dùng cho giao tiếp I2C
#include <EEPROM.h> // EEPROM nội giả lập trên flash của ESP32

// Định nghĩa loại EEPROM sử dụng
#define eeprom_on_arduino 0 // EEPROM nội bộ (giả lập trong flash)
#define eeprom_on_ic 1      // EEPROM ngoài giao tiếp I2C

class MyEEPROM
{
public:
  // Khởi tạo cho EEPROM nội (ESP32)
  MyEEPROM(uint32_t size_in_byte);

  // Khởi tạo cho EEPROM ngoài (IC 24Cxx)
  MyEEPROM(uint32_t size_m, byte deviceAddress);

  // Hàm khởi tạo EEPROM
  void begin();

  // Ghi 1 byte vào địa chỉ chỉ định (EEPROM nội hoặc ngoài tùy cấu hình)
  void write_1_byte(uint32_t address, byte data);

  // Đọc 1 byte tại địa chỉ chỉ định
  byte read_1_byte(uint32_t address);

  // Ghi nhiều byte liên tiếp
  int WriteMultiByte(uint16_t data_addr, const byte *buff_data, int quantity);

  // Đọc nhiều byte liên tiếp
  int ReadMultiByte(uint16_t data_addr, byte *buff_data, int quantity);

  // Hiển thị toàn bộ nội dung bộ nhớ
  void ShowMem(Stream &_stream);

  // Xoá toàn bộ bộ nhớ (ghi 0xFF)
  void FormatMem(Stream &_stream);

  // Kiểm tra khả năng đọc/ghi bộ nhớ
  bool TestMem(Stream &_stream);

private:
  // Ghi 1 byte vào EEPROM ngoài (I2C)
  void write_byte_eeprom_ic(uint32_t address, byte data);

  // Đọc 1 byte từ EEPROM ngoài (I2C)
  byte read_byte_eeprom_ic(uint32_t address);

  // Ghi 1 byte vào EEPROM nội
  void write_byte_eeprom_arduino(uint32_t address, byte data);

  // Đọc 1 byte từ EEPROM nội
  byte read_byte_eeprom_arduino(uint32_t address);

  byte select_eeprom;   // Biến chọn loại EEPROM (nội hay ngoài)
  uint32_t size_bytes;  // Tổng số byte bộ nhớ sử dụng
  uint32_t size_memory; // Dung lượng EEPROM ngoài (đơn vị Kbit)
  byte m_deviceAddress; // Địa chỉ I2C của EEPROM ngoài
};

#endif