#pragma once
#include <ModbusMaster.h>

void InitRS485();
void updateModbus();
float getTemp();
float getHumi();