#include "ATCommand.h"
#include <string.h>
#include <Preferences.h>
#include <Arduino.h>

String device_serial = "";
String device_pcb = "";
String device_hw = "";

Preferences pref;

#define AT_OK "OK"
#define AT_ERROR "ERROR"

void Command_Process(Stream &_stream, char *cmd, int len)
{
    // Loại bỏ ký tự \r và \n ở cuối chuỗi
    while (len > 0 && (cmd[len-1] == '\r' || cmd[len-1] == '\n'))
        cmd[--len] = 0;
    // AT hoặc at
    if (strcmp(cmd, "AT") == 0 || strcmp(cmd, "at") == 0)
    {
        _stream.println(AT_OK);
    }
    // AT+HELP hoặc at+help
    else if (strcmp(cmd, "AT+HELP") == 0 || strcmp(cmd, "at+help") == 0)
    {
        _stream.println("Danh sach lenh:");
        _stream.println("AT / at");
        _stream.println("AT+HELP / at+help");
        _stream.println("AT+SERIAL? / at+serial?");
        _stream.println("AT+SERIAL=xxx / at+serial=xxx");
        _stream.println("AT+PCB? / at+pcb?");
        _stream.println("AT+PCB=xxx / at+pcb=xxx");
        _stream.println("AT+HW? / at+hw?");
        _stream.println("AT+HW=xxx / at+hw=xxx");
        _stream.println("AT+INFO / at+info");
        _stream.println("AT+RESTART / at+restart");
        _stream.println(AT_OK);
    }
    // AT+INFO hoặc at+info
    else if (strcmp(cmd, "AT+INFO") == 0 || strcmp(cmd, "at+info") == 0)
    {
        pref.begin("DEVICE", true);
        String sn  = pref.getString("Serial", device_serial);
        String pcb = pref.getString("PCBVersion", device_pcb);
        String hw  = pref.getString("HWVersion", device_hw);
        pref.end();
        _stream.printf("\r\nSerial Number: %s\r\n", sn.c_str());
        _stream.printf("PCB Version: %s\r\n", pcb.c_str());
        _stream.printf("HW Version: %s\r\n", hw.c_str());
        _stream.println(AT_OK);
    }
    // Đọc serial number
    else if (strcmp(cmd, "AT+SERIAL?") == 0 || strcmp(cmd, "at+serial?") == 0)
    {
        pref.begin("DEVICE", true);
        String sn = pref.getString("Serial", device_serial);
        pref.end();
        _stream.printf("\r\nSerial Number: %s\r\n%s", sn.c_str(), AT_OK);
    }
    // Ghi serial number
    else if (strncmp(cmd, "AT+SERIAL=", 10) == 0 || strncmp(cmd, "at+serial=", 10) == 0)
    {
        char *val = cmd + 10;
        device_serial = String(val);
        pref.begin("DEVICE", false);
        pref.putString("Serial", device_serial);
        pref.end();
        _stream.printf("\r\nSet Serial Number = %s\r\n%s", device_serial.c_str(), AT_OK);
    }
    // Đọc PCB version
    else if (strcmp(cmd, "AT+PCB?") == 0 || strcmp(cmd, "at+pcb?") == 0)
    {
        pref.begin("DEVICE", true);
        String pcb = pref.getString("PCBVersion", device_pcb);
        pref.end();
        _stream.printf("\r\nPCB Version: %s\r\n%s", pcb.c_str(), AT_OK);
    }
    // Ghi PCB version
    else if (strncmp(cmd, "AT+PCB=", 7) == 0 || strncmp(cmd, "at+pcb=", 7) == 0)
    {
        char *val = cmd + 7;
        device_pcb = String(val);
        pref.begin("DEVICE", false);
        pref.putString("PCBVersion", device_pcb);
        pref.end();
        _stream.printf("\r\nSet PCB Version = %s\r\n%s", device_pcb.c_str(), AT_OK);
    }
    // Đọc HW version
    else if (strcmp(cmd, "AT+HW?") == 0 || strcmp(cmd, "at+hw?") == 0)
    {
        pref.begin("DEVICE", true);
        String hw = pref.getString("HWVersion", device_hw);
        pref.end();
        _stream.printf("\r\nHW Version: %s\r\n%s", hw.c_str(), AT_OK);
    }
    // Ghi HW version
    else if (strncmp(cmd, "AT+HW=", 6) == 0 || strncmp(cmd, "at+hw=", 6) == 0)
    {
        char *val = cmd + 6;
        device_hw = String(val);
        pref.begin("DEVICE", false);
        pref.putString("HWVersion", device_hw);
        pref.end();
        _stream.printf("\r\nSet HW Version = %s\r\n%s", device_hw.c_str(), AT_OK);
    }
    // Lệnh restart
    else if (strcmp(cmd, "AT+RESTART") == 0 || strcmp(cmd, "at+restart") == 0)
    {
        _stream.println("\r\nRestarting...\r\n");
        _stream.println(AT_OK);
        delay(100);
        ESP.restart();
    }
    // Lệnh chưa hỗ trợ
    else
    {
        _stream.printf("\r\n%s: Unknown AT command\r\n", AT_ERROR);
    }
}