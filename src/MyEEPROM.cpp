// File: MyEEPROM.cpp - Xử lý đọc/ghi EEPROM nội bộ và EEPROM ngoài (IC 24Cxx)
#include "MyEEPROM.h"
#include <EEPROM.h>
#include <Wire.h>

// Hàm khởi tạo cho EEPROM nội bộ (EEPROM giả lập flash trên ESP32)
MyEEPROM::MyEEPROM(uint32_t size_in_byte)
{
  select_eeprom = eeprom_on_arduino; // Chọn EEPROM nội bộ
  size_bytes = size_in_byte;         // Thiết lập kích thước vùng nhớ
}

// Hàm khởi tạo cho EEPROM ngoài (IC 24Cxx)
MyEEPROM::MyEEPROM(uint32_t size_m, byte deviceAddress)
{
  m_deviceAddress = deviceAddress; // Địa chỉ I2C của EEPROM ngoài
  size_memory = size_m;            // Kích thước theo đơn vị Kbit
  size_bytes = size_m * 128;       // Đổi Kbit sang byte
  select_eeprom = eeprom_on_ic;    // Chọn EEPROM ngoài
}

// Khởi tạo bộ nhớ EEPROM tùy theo loại
void MyEEPROM::begin()
{
  if (select_eeprom == eeprom_on_arduino)
  {
    EEPROM.begin(size_bytes); // EEPROM nội cần gọi begin()
  }
}

// Ghi 1 byte vào EEPROM ngoài (I2C)
void MyEEPROM::write_byte_eeprom_ic(uint32_t address, byte data)
{
  if ((address < size_bytes) && (read_1_byte(address) != data))
  {
    if (size_memory <= 2)
    {
      // EEPROM nhỏ: 24C01, 24C02
      Wire.beginTransmission(m_deviceAddress);
      Wire.write((byte)address);
      Wire.write(data);
      Wire.endTransmission();
      delayMicroseconds(1500);
    }
    else if (size_memory <= 16)
    {
      // EEPROM vừa: 24C04, 24C08, 24C16 (chia theo page)
      Wire.beginTransmission((byte)(m_deviceAddress | ((address >> 8) & 0x07)));
      Wire.write(address & 0xFF);
      Wire.write(data);
      Wire.endTransmission();
      delayMicroseconds(3500);
    }
    else if (size_memory <= 256)
    {
      // EEPROM lớn: 24C32, 24C64, 24C128...
      Wire.beginTransmission(m_deviceAddress);
      Wire.write(address >> 8);
      Wire.write(address & 0xFF);
      Wire.write(data);
      Wire.endTransmission();
      delayMicroseconds(1500);
    }
  }
}

// Đọc 1 byte từ EEPROM ngoài (I2C)
byte MyEEPROM::read_byte_eeprom_ic(uint32_t address)
{
  byte data = 0;
  if (address < size_bytes)
  {
    if (size_memory <= 2)
    {
      Wire.beginTransmission(m_deviceAddress);
      Wire.write(address);
      Wire.endTransmission();
      Wire.requestFrom(m_deviceAddress, (byte)1);
    }
    else if (size_memory <= 16)
    {
      Wire.beginTransmission((byte)(m_deviceAddress | ((address >> 8) & 0x07)));
      Wire.write(address & 0xFF);
      Wire.endTransmission();
      Wire.requestFrom((byte)(m_deviceAddress | ((address >> 8) & 0x07)), (byte)1);
    }
    else if (size_memory <= 256)
    {
      Wire.beginTransmission(m_deviceAddress);
      Wire.write(address >> 8);
      Wire.write(address & 0xFF);
      Wire.endTransmission();
      Wire.requestFrom(m_deviceAddress, (byte)1);
    }
    if (Wire.available())
      data = Wire.read();
  }
  return data;
}

// Ghi 1 byte vào EEPROM nội bộ
void MyEEPROM::write_byte_eeprom_arduino(uint32_t address, byte data)
{
  if ((address < size_bytes) && (read_1_byte(address) != data))
  {
    EEPROM.write(address, data);
    // Không commit ở đây nếu ghi nhiều byte
  }
}

// Đọc 1 byte từ EEPROM nội bộ
byte MyEEPROM::read_byte_eeprom_arduino(uint32_t address)
{
  return EEPROM.read(address);
}

// Ghi 1 byte tùy loại EEPROM được chọn
void MyEEPROM::write_1_byte(uint32_t address, byte data)
{
  if (select_eeprom == eeprom_on_arduino)
  {
    write_byte_eeprom_arduino(address, data);
  }
  else
  {
    write_byte_eeprom_ic(address, data);
  }
}

// Đọc 1 byte tùy loại EEPROM được chọn
byte MyEEPROM::read_1_byte(uint32_t address)
{
  if (select_eeprom == eeprom_on_arduino)
  {
    return read_byte_eeprom_arduino(address);
  }
  else
  {
    return read_byte_eeprom_ic(address);
  }
  return 0;
}

// Đọc nhiều byte liên tiếp từ EEPROM
int MyEEPROM::ReadMultiByte(uint16_t data_addr, byte *buff_data, int quantity)
{
  delayMicroseconds(3000); // Chờ sẵn sàng sau ghi
  for (int i = 0; i < quantity; i++)
  {
    buff_data[i] = read_1_byte(data_addr++);
  }
  return quantity;
}

// Ghi nhiều byte liên tiếp vào EEPROM
int MyEEPROM::WriteMultiByte(uint16_t data_addr, const byte *buff_data, int quantity)
{
  for (int i = 0; i < quantity; i++)
  {
    write_1_byte(data_addr, *buff_data);
    ++buff_data;
    ++data_addr;
  }
  // Chỉ commit nếu là EEPROM nội (ESP32)
  if (select_eeprom == eeprom_on_arduino)
    EEPROM.commit();
  return quantity;
}

// Xoá toàn bộ EEPROM (ghi 0xFF toàn bộ)
void MyEEPROM::FormatMem(Stream &_stream)
{
  uint32_t test_time = millis();
  _stream.printf("\r\nFormat %s EEPROM, memory size=%u bytes", (select_eeprom == eeprom_on_arduino ? "internal" : "external"), size_bytes);
  for (int i = 0; i < size_bytes; i++)
  {
    if (i % 32 == 0)
      _stream.print("\r\n");
    write_1_byte(i, 0xFF);
    _stream.print("FF ");
  }
  if (select_eeprom == eeprom_on_arduino)
    EEPROM.commit();
  test_time = millis() - test_time;
  _stream.printf("\r\nCompleted in %u(ms)\r\n", test_time);
}

// Hiển thị nội dung bộ nhớ EEPROM
void MyEEPROM::ShowMem(Stream &_stream)
{
  uint32_t test_time = millis();
  _stream.printf("\r\nShow %s EEPROM, memory size=%u bytes", (select_eeprom == eeprom_on_arduino ? "internal" : "external"), size_bytes);
  for (int i = 0; i < size_bytes; i++)
  {
    if (i % 32 == 0)
      _stream.print("\r\n");
    _stream.printf("%02X ", read_1_byte(i));
  }
  test_time = millis() - test_time;
  _stream.printf("\r\nCompleted in %u(ms)\r\n", test_time);
}

// Kiểm tra khả năng đọc/ghi EEPROM
bool MyEEPROM::TestMem(Stream &_stream)
{
  bool retOK = true;
  bool testOK;
  uint8_t buffW[] = "012345678901234567890123456789012345678901"; // 42 byte mẫu test
  uint8_t buffB[33];                                              // Buffer lưu dữ liệu gốc
  uint8_t buffR[33];                                              // Buffer đọc lại
  uint32_t test_time = millis();

  _stream.printf("\r\nTest %s EEPROM, memory size=%u bytes", (select_eeprom == eeprom_on_arduino ? "internal" : "external"), size_bytes);

  for (int i = 0; i < (size_bytes / 32); i++)
  {
    ReadMultiByte(32 * i, buffB, 32);           // Lưu dữ liệu gốc
    WriteMultiByte(32 * i, &buffW[i % 10], 32); // Ghi chuỗi test
    memset(buffR, 0, 33);
    ReadMultiByte(32 * i, buffR, 32);                  // Đọc lại
    testOK = (memcmp(&buffW[i % 10], buffR, 32) == 0); // So sánh
    if (!testOK)
      retOK = false;
    WriteMultiByte(32 * i, buffB, 32); // Khôi phục dữ liệu
    _stream.printf("\r\nRead/Write(%04X-%04X)=%s - %s (%ums)", 32 * i, 32 * (i + 1) - 1, buffR, testOK ? "OK" : "NG", millis() - test_time);
  }

  test_time = millis() - test_time;
  _stream.printf("\r\nTest completed in %u(ms). Result=%s\r\n", test_time, retOK ? "OK" : "NG");
  return retOK;
}