#pragma once

#include <Arduino.h>

#define FORMAT_LITTLEFS_IF_FAILED true

int initFileSystem();
void checkAvailableFlashSpace();
void listDir(const char * dirname, uint8_t levels);
void writeFile(const char * path, const char * message);
void readFile(const char * path);
