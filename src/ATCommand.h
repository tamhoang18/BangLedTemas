#ifndef ATCOMMAND_H
#define ATCOMMAND_H

#include <Arduino.h>

#define AT_BUFFER_SIZE 128

void Command_Process(Stream &_stream, char *cmd, int len);

#endif