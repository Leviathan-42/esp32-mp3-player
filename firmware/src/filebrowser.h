#pragma once
#include <Arduino.h>

#define MAX_FILES 128
#define MAX_PATH  64

extern char   fileList[MAX_FILES][MAX_PATH];
extern int    fileCount;

// Scan SD card root for .mp3 files. Returns number found.
int scanSD();
