#include "RS485.h"

#define RS485_RX 16
#define RS485_TX 17
#define Control_RS485 4

static float temp = 0.0;
static float humi = 0.0;
ModbusMaster modbus;

void InitRS485()
{
    pinMode(Control_RS485, OUTPUT);
    digitalWrite(Control_RS485, LOW);
    Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
    modbus.begin(1, Serial2);
    modbus.preTransmission([]()
                           { digitalWrite(Control_RS485, HIGH); });
    modbus.postTransmission([]()
                            { digitalWrite(Control_RS485, LOW); });
}

void updateModbus() {
    uint8_t result = modbus.readInputRegisters(0x0000, 2);
    if (result == modbus.ku8MBSuccess) {
        temp = modbus.getResponseBuffer(0) / 100.0;
        humi = modbus.getResponseBuffer(1) / 100.0;

        //Serial.printf("[MODBUS] Temp: %.1f °C, Humi: %.1f %%\n", temp, humi);
    } else {
        //Serial.printf("[MODBUS] Error code: 0x%02X\n", result);
    }
}

float getTemp() { return temp; }
float getHumi() { return humi; }
